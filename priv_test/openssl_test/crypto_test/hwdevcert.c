#include "swapi.h"
#include "swapp.h"
#include "swthrd.h"
#include "swlog.h"
#include "hwiptv_priv.h"
#include "swparameter.h"
#include "hwdevcert.h"
#include "swmutex.h"
#include <x509.h>
#include <x509v3.h>
#include <x509_vfy.h>
#include <safestack.h>
#include <ssl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STBROOTCERT          "/usr/local/etc/setting/X509/stbrootca.pem"
#define MIDLECERT            "/usr/local/etc/setting/X509/stbmcert.pem"
#define MIDLECERTPRIVATA     "/usr/local/etc/setting/X509/stbmkey.pem"
#define MIDLECERTCHAIN       "/usr/local/etc/setting/X509/stbmchain.pem"

#define DEVCERT              "/var/x509/devcert.pem"
#define DEVCERTKEY           "/var/x509/devcertkey.pem"
#define TMP_KEY              "/tmp/tmp_key.pem"

#define RSA_KEY_BITS         2048
#define RAND_NUMBER_LEN      16
#define LIMITE_TIME          7210 //天为单位
#define SERIAL_NUMBER        13148 //随便，不超过20bit即可
#define CHECK_SUCCESS        1 //1 为成功，0为失败
#define ENCRYPT_DATA         0
#define DECRYPT_DATA         1

static HANDLE m_devcert_thrd = NULL;
static bool m_cer_status = false; //记录证书安装的状态,可用为true
static EVP_PKEY *m_key_buf = NULL; //只初始化一次

extern int swsyscmd(const char* cmd );
extern bool hw_time_get_state(void);
extern bool sw_get_dvb_sync_time_state(void);

typedef struct subj_info_t
{
	char *name;
	char *info;
}subj_info_t;

typedef struct ext_info_t
{
	int  nid;
	char *info;
}ext_info_t;

bool sw_devcert_install_ok(void)
{
	return m_cer_status;
}

static void callback(int p, int n, void *arg)
{
	char c = 'B'; 
	if (p == 0)
		c = '.';
	if (p == 1)
		c = '+';
	if (p == 2)
		c = '*';
	if (p == 3)
		c = '\n';
	HWIPTV_LOG_DEBUG("%c",c);
}

/*r2r芯片加解密数据转换接口
 in_data[in] 需要做处理的数据存放地址
 insize   in 传入数据大小,加密的数据要8字节对齐的大小，解密的文件数据就是文件的大小
 out_data[out] 处理后数据存放地址
 outsize  out地址内存大小
 return 成功返回输出的数据大小
 flag   ENCRYPT_DATA == 加密in处的数据, DECRYPT_DATA== 解密数据,其他值无效
*/

static int trans_data_r2r(char *in_data,int insize,char *out_data,int outsize, int flag)
{
	if(in_data == NULL || out_data == NULL)
		return false;
	int ret = -1;
	//在初始化时(vmx模块初始化之前)完成通道占用
	//swplatform_r2r_init(); //海思反馈,这里会影响vmx的加密使用，如果我们需要用则，要一直占用一个通道
	if(flag == ENCRYPT_DATA)
	{
		ret=swplatform_r2r_encrypt(in_data,insize,out_data,outsize);
		if(ret <= 0)
			HWIPTV_LOG_ERROR("fail to encry data\n");
	}
	else if(flag == DECRYPT_DATA)
	{

		ret= swplatform_r2r_decrypt(in_data,insize,out_data,outsize);
		if(ret <= 0)
			HWIPTV_LOG_ERROR("fail to decry data\n");

	}
	//swplatform_r2r_deinit();
	return ret;
}
static bool get_data_from_file(char *filename,int file_size,char *data_buf,int buf_len)
{
	if(filename == NULL || data_buf == NULL || file_size > buf_len)
		return false;

	FILE *fp = fopen(filename,"r");
	if(fp == NULL)
	{
		HWIPTV_LOG_ERROR("faile to open %s\n",filename);
		return false;
	}
	if(fread(data_buf,sizeof(char),file_size,fp) != file_size)
	{
		HWIPTV_LOG_ERROR("faile to read %s\n",DEVCERTKEY);
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

static bool write_data_to_file(char *filename,char *data_buf,int data_len)
{
	if(filename == NULL || data_buf == NULL)
		return false;
	FILE *fp = fopen(filename,"w");
	if(fp == NULL)
	{
		HWIPTV_LOG_ERROR("faile to open %s\n",filename);
		return false;
	}
	if(fwrite(data_buf,sizeof(char),data_len,fp) != data_len)
	{
		HWIPTV_LOG_ERROR("faile to write %s\n",DEVCERTKEY);
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

//转换文件数据(R2R加解密)
//flag ENCRYPT_DATA = 加密，DECRYPT_DATA == 解密
static bool trans_file_data(char *in_file,char *out_file,int flag)
{
	if(in_file == NULL || out_file == NULL)
		return false;
	char *key_buf = NULL;
	char *end_buf = NULL;
	int data_size = 0;
	int reser = 16; //保留字节
	bool ret = false;
	struct stat file_stat;
	sw_memset(&file_stat,sizeof(file_stat),0,sizeof(file_stat));
	if(lstat(in_file,&file_stat) != 0)
	{
		HWIPTV_LOG_ERROR("file %s not exist\n",in_file);
		return false;
	}
	data_size = file_stat.st_size;

	key_buf = (char *)malloc(data_size +reser);
	end_buf = (char *)malloc(data_size+reser);
	if(key_buf == NULL || end_buf == NULL)
	{
		HWIPTV_LOG_ERROR("fail to malloc mem\n");
		goto END;
	}
	sw_memset(key_buf,data_size+reser,0,data_size+reser);
	sw_memset(end_buf,data_size+reser,0,data_size+reser);
	//读数据
	if(!get_data_from_file(in_file,file_stat.st_size,key_buf,data_size+reser))
		goto END;
	//加解密
	data_size = trans_data_r2r(key_buf,data_size,end_buf,data_size+reser,flag);
	if(data_size <=0)
		goto END;
	//保存到文件
	if(!write_data_to_file(out_file,end_buf,data_size))
		goto END;
	ret = true;
END:
	if(key_buf)
		free(key_buf);
	key_buf = NULL;
	if(end_buf)
		free(end_buf);
	end_buf = NULL;
	return ret;
}

//获取内存中的私钥
void *sw_dev_private_key_get(void)
{
	//证书解密私钥
	return m_key_buf;
}
//初始化证书私钥，将私钥加载到内存;
static bool dev_private_key_init(void)
{
	if(m_key_buf)
	{
		HWIPTV_LOG_DEBUG("private key has been init\n");
		return true;
	}
	//转换加密文件
	struct stat file_stat;
	sw_memset(&file_stat,sizeof(file_stat),0,sizeof(file_stat));
	if(lstat(DEVCERTKEY,&file_stat) != 0)
	{
		HWIPTV_LOG_ERROR("file %s not exist\n",DEVCERTKEY);
		return false;
	}
	if(!trans_file_data(DEVCERTKEY,DEVCERTKEY_TMP,DECRYPT_DATA))
	{
		HWIPTV_LOG_ERROR("fail to decrypt file %s\n",DEVCERTKEY);
		return false;
	}
	FILE *priv_fp = fopen(DEVCERTKEY_TMP,"r"); 
	if(priv_fp == NULL)
	{
		HWIPTV_LOG_ERROR("fail to open %s\n",DEVCERTKEY_TMP);
		goto END;
	}
	m_key_buf = PEM_read_PrivateKey(priv_fp,NULL,NULL,NULL);
	if(m_key_buf == NULL)
		HWIPTV_LOG_DEBUG("fail to read %s\n",DEVCERTKEY_TMP);
END:
	if(priv_fp)
		fclose(priv_fp);
	priv_fp = NULL;
	remove(DEVCERTKEY_TMP);
	return (m_key_buf ? true: false);
}

/*
  @brief 获取证书请求模板
  pk 生成模板需要的public key
  req[out] 返回生的请求模板的指针 
 
*/
static bool set_dev_req_info(EVP_PKEY *pk,X509_REQ *req)
{
	if(pk == NULL || req == NULL)
	{
		HWIPTV_LOG_DEBUG("pclient_key is null\n");
		return false;
	}
	X509_NAME *name = NULL;
	char own_info[64] = {0};
	subj_info_t subj_info[] = {{"C","CN"},{"ST","GD"},{"O","Huawei"},{"OU","STB PDU"}};
	X509_REQ_set_pubkey(req,pk);
	name = X509_REQ_get_subject_name(req);
	if(name == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to get X509_REQ_get_subject_name \n");
		return false;
	}

	int i = 0;
	for(i=0;i< sizeof(subj_info)/sizeof(subj_info[0]);i++) //添加固定的信息
		X509_NAME_add_entry_by_txt(name,subj_info[i].name, MBSTRING_ASC, (const unsigned char *)subj_info[i].info, -1, -1, 0);

	sw_parameter_get("serial",own_info, sizeof(own_info));//获取盒子的SN号
	//添加因设备而变的信息
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (const unsigned char *)own_info, -1, -1, 0);

	//签名
	if(X509_REQ_sign(req,pk,EVP_sha256())<=0)
		return false;
	return true;
}

//添加X509V3中的拓展信息
static int add_ext(X509 *mdcert,X509 *rqcert, int nid, char *value)
{
    X509_EXTENSION *ex = NULL;
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, mdcert, rqcert, NULL, NULL, 0);
    ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
    if (!ex)
        return 0;

    X509_add_ext(rqcert, ex, -1);
    X509_EXTENSION_free(ex);
    return 1;
}

/*
  @brief对申请的证书模板使用中间证书以及中间私钥进行签名
 
*/
static bool sig_dev_by_middlecert( X509_REQ *req,X509 *dev_cert,int days,int sn)
{

	if(dev_cert == NULL || req == NULL)
		return false;
	bool result = false;
	FILE *fp = NULL;
	X509 *md_cert = NULL;
	FILE *key_fp = NULL;
	EVP_PKEY *md_key = NULL;
	int i = 0;

	X509_set_version(dev_cert,2);
	ASN1_INTEGER_set(X509_get_serialNumber(dev_cert),sn);
	X509_gmtime_adj(X509_get_notBefore(dev_cert),0);
	X509_gmtime_adj(X509_get_notAfter(dev_cert),(long)days*60*60*24);
	X509_set_subject_name(dev_cert,X509_REQ_get_subject_name(req));
	X509_set_pubkey(dev_cert,X509_REQ_get_pubkey(req));

	//从flase中获取中间证书
	fp = fopen(MIDLECERT, "r");
	if(fp == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to open %s\n",MIDLECERT);
		goto END;
	}
	md_cert = PEM_read_X509(fp, NULL, NULL, NULL);
	if(md_cert == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to read %s\n",MIDLECERT);
		goto END;
	}
	X509_set_issuer_name(dev_cert,X509_get_subject_name(md_cert));
	//获取中间证书中的key,先解密到临时文件中
	if(!trans_file_data(MIDLECERTPRIVATA,TMP_KEY,DECRYPT_DATA))
	{
		HWIPTV_LOG_ERROR("fail to decrypt file %s\n",MIDLECERTPRIVATA);
	}
	key_fp = fopen(TMP_KEY,"r");
	if(key_fp == NULL)
	{
		HWIPTV_LOG_ERROR("fail to open %s\n",TMP_KEY);
		goto END;
	}
	md_key = PEM_read_PrivateKey(key_fp, NULL, NULL, NULL);
	if(md_key == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to read %s\n",MIDLECERTPRIVATA);
		goto END;
	}
	//添加X509拓展信息
	ext_info_t ext_info[] = { {NID_basic_constraints,"CA:FALSE"},{ NID_netscape_comment,"OpenSSL EC2108CV5 Certificate"},
		                    {NID_subject_key_identifier,"hash"},{NID_authority_key_identifier,"keyid:always"}};
	for( i = 0; i < (sizeof(ext_info)/sizeof(ext_info[0])); i++)
		add_ext(md_cert,dev_cert,ext_info[i].nid,ext_info[i].info);

	//设置签名
	if(X509_sign(dev_cert,md_key,EVP_sha256())<=0)
	{
		HWIPTV_LOG_DEBUG("error X509_sign......\n");
		goto END;
	}
	result = true;

END:
	if(fp)
		fclose(fp);
	if(key_fp)
		fclose(key_fp);
	if(md_key)
		EVP_PKEY_free(md_key);
	if(md_cert)
		X509_free(md_cert);
	fp = NULL;
	key_fp = NULL;
	md_key = NULL;
	md_cert = NULL;
	if(access(TMP_KEY,F_OK) == 0)
		remove(TMP_KEY);
	return result;;
}

/*
  @brief 加密密钥对，保存到flash中
 
*/
static bool save_dev_private_key(EVP_PKEY *key)
{
	if(key == NULL)
		return false;
	bool ret = false;
	if(access("/var/x509", F_OK) != 0)
	{
		mkdir("/var/x509",0711);
	}
	//保存到tmp,最后加密后保存
	FILE *fp = fopen(TMP_KEY,"wb+");
	if(fp == NULL)
	{
		HWIPTV_LOG_DEBUG("faile to open  %s\n",TMP_KEY);
		return false;
	}
	PEM_write_PrivateKey(fp, key, NULL, NULL, 0, NULL, NULL);

	fclose(fp);
	fp = NULL;
	//加密保存
	if(!trans_file_data(TMP_KEY,DEVCERTKEY,ENCRYPT_DATA))
	{
		HWIPTV_LOG_ERROR("fail to encrypt file %s\n",TMP_KEY);
		ret = false;
	}
	else
		ret = true;
	remove(TMP_KEY);
	return ret;
}


static X509_STORE *m_ca_store = NULL; //校验证书的证书链
//校验客户证书是否由上级证书签发,成功返回true，否者返回失败,然后会重新生成
static bool verify_cert(char *r_cert,char *c_cert )
{
	bool result = false;
	FILE *fp = NULL;
	X509 *ca_cert = NULL;
	X509 *us_cert = NULL;
	int ret = 0;
	X509_STORE_CTX *ctx = X509_STORE_CTX_new();
	if(ctx == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to init X509_STORE_CTX_new\n");
		goto END;
	}

	//读取用于签名的证书
	fp = fopen(r_cert,"r");
	if(fp == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to open the  file  %s\n",r_cert);
		goto END;
	}
	ca_cert = PEM_read_X509(fp, NULL, NULL, NULL);
	if(ca_cert == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to read %s\n",r_cert);
		goto END;
	}
	fclose(fp);
	fp = NULL;

	//int ret = 1;
	ret = X509_STORE_add_cert(m_ca_store,ca_cert);
	if(ret != 1)
	{
		HWIPTV_LOG_DEBUG("X509_STORE_add_cert ret = %d\n",ret);
		goto  END;
	}
	//读取被前面的证书
	fp = fopen(c_cert,"r");
	if(fp == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to open the  file  %s\n",c_cert);
		goto END;
	}
	us_cert = PEM_read_X509(fp, NULL, NULL, NULL);
	if(us_cert == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to read %s\n",c_cert);
		goto END;
	}
	fclose(fp);
	fp = NULL;

	X509_STORE_set_flags(m_ca_store, X509_V_FLAG_CRL_CHECK_ALL); //所有的项目都要校验
	ret = X509_STORE_CTX_init(ctx, m_ca_store, us_cert, NULL);
	if(ret != 1)
	{
		HWIPTV_LOG_DEBUG("fail to init  X509_STORE_CTX_init ret = %d\n",ret);
		goto  END;
	}
	ret = X509_verify_cert(ctx);
	if(ret != CHECK_SUCCESS)
	{
		HWIPTV_LOG_DEBUG("fail to X509_verify_cert ret = %d\n",ret);
		goto  END;
	}
	result = true;

END:
	if(fp)
		fclose(fp);
	fp = NULL;
	if(ca_cert)
		X509_free(ca_cert);
	if(us_cert)
		X509_free(us_cert);
	if(ctx)
	{
		X509_STORE_CTX_cleanup(ctx);
		X509_STORE_CTX_free(ctx);
	}
	return result;
}

//保存证书到flash
static bool save_dev_cert(X509 *cert,char *file )
{
	if(cert == NULL || file == NULL)
		return false;
	if(access("/var/x509", F_OK) != 0)
	{
		mkdir("/var/x509",0711);
	}

	FILE *cert_fp = fopen(file,"wb");
	if(cert_fp == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to open %s\n",file);
		return false;
	}
	//件数据写到文件中
	//X509_print_fp(cert_fp, cert);
	PEM_write_X509(cert_fp,cert);
	fclose(cert_fp);
	cert_fp = NULL;
	return  true;
}
static bool encrypt_data( EVP_PKEY *pkey,const unsigned char *inbuf,unsigned char *outbuf )
{
	int ret  = 0;
	if(pkey == NULL || inbuf == NULL || outbuf == NULL)
		return false;
	if(pkey->pkey.rsa)
	{
		int keysize = RSA_size(pkey->pkey.rsa);
		ret = RSA_private_encrypt(keysize,inbuf,outbuf,(pkey->pkey.rsa),RSA_NO_PADDING);
		if(ret <= 0)
		{
			HWIPTV_LOG_DEBUG("fail to encrypt data \n");
			return false;
		}
	}
	return true;
}
static bool decrypt_data( EVP_PKEY *pkey,const unsigned char *inbuf,unsigned char *outbuf )
{
	int ret  = 0;
	if(pkey == NULL || inbuf == NULL || outbuf == NULL)
		return false;
	if(pkey->pkey.rsa)
	{
		int keysize = RSA_size(pkey->pkey.rsa);
		ret =  RSA_public_decrypt(keysize,inbuf,outbuf,(pkey->pkey.rsa),RSA_NO_PADDING);
		if(ret <= 0)
		{
			HWIPTV_LOG_DEBUG("fail to encrypt data \n");
			return false;
		}
	}
	return true;
}

//生成128bits的随机数，验证是否预置成功
static bool is_install_x509_success(void)
{
	EVP_PKEY *private_key = NULL;
	EVP_PKEY *public_key = NULL;
	FILE *priv_fp = NULL;
	FILE *pul_fp = NULL;
	X509 *cert = NULL;
	char randbuf[256] = {0};
	char decbuf[264] = {0};
	unsigned char encbuf[264] = {0};

	int len = 0;
	time_t seed = time(NULL);
	int rand_num = 0;
	bool result = false;
	m_ca_store = X509_STORE_new();
	if(m_ca_store == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to X509_STORE_new\n");
		goto END;;
	}
	if((access(DEVCERT, F_OK) != 0) || (access(DEVCERTKEY, F_OK) != 0))
	{
		HWIPTV_LOG_DEBUG("some file not exist\n");
		goto END;
	}

	if(!verify_cert(STBROOTCERT,MIDLECERT))
	{
		HWIPTV_LOG_DEBUG("fail to fail to verify %s ,%s\n",STBROOTCERT,MIDLECERT);
		goto END;
	}

	if(!verify_cert(MIDLECERT,DEVCERT))
	{
		HWIPTV_LOG_DEBUG("fail to fail to verify %s ,%s\n",MIDLECERT,DEVCERT);
		goto END;
	}

	for(len = 0; len < RAND_NUMBER_LEN;)
	{
		rand_num = 10000000 + rand_r((unsigned int *)&seed)%(89999999); //每次取8位
		len += snprintf(randbuf+len,sizeof(randbuf),"%d",rand_num);
	}
	//先将flash中的密钥解密到临时文件
	if(!trans_file_data(DEVCERTKEY,DEVCERTKEY_TMP,DECRYPT_DATA))
	{
		HWIPTV_LOG_ERROR("fail to init enc key\n");
		goto END;
	}
	priv_fp = fopen(DEVCERTKEY_TMP,"r"); 
	if(priv_fp == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to open %s\n",DEVCERTKEY_TMP);
		goto END;
	}
	private_key = PEM_read_PrivateKey(priv_fp,NULL,NULL,NULL);
	if(private_key == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to read %s\n",DEVCERTKEY_TMP);
		goto END;
	}

	pul_fp = fopen(DEVCERT,"r");
	if(pul_fp == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to open %s\n",DEVCERT);
		goto END;
	}
	cert = PEM_read_X509(pul_fp,NULL,NULL,NULL);
	if(cert == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to read %s\n",DEVCERT);
		goto END;
	}
	public_key = X509_get_pubkey(cert);
	if(public_key == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to get public_key\n");
		goto END;
	}
	X509_free(cert);

	if(!encrypt_data(private_key,(const unsigned char *)randbuf,encbuf))
	{
		goto END;
	}
	if(!decrypt_data(public_key,encbuf,(unsigned char *)decbuf))
	{
		goto END;
	}
	if(strlen(decbuf) != RAND_NUMBER_LEN)
		goto END;

	if(strncmp(randbuf,decbuf,RAND_NUMBER_LEN) !=0)
		goto END;
	result = true;
END:
	if(access(DEVCERTKEY_TMP,F_OK)==0)
		remove(DEVCERTKEY_TMP);
	if(priv_fp)
	{
		fclose(priv_fp);
		priv_fp = NULL;
	}
	if(pul_fp)
	{
		fclose(pul_fp);
		pul_fp = NULL;
	}
	if(private_key)
	{
		EVP_PKEY_free(private_key);
		private_key = NULL;
	}
	if(public_key)
	{
		EVP_PKEY_free(public_key);
		public_key = NULL;
	}
	if(m_ca_store)
	{
		X509_STORE_free(m_ca_store);
		m_ca_store = NULL;
	}
	return result;	
}

static bool x509_install_proc(uint32_t wparam, uint32_t lparam)
{
	//等待时间同步成功,在系统时间不正确的情况下,做出来的证书或证书校验不对
	if(!hw_time_get_state() && !sw_get_dvb_sync_time_state())
	{
		sw_thrd_delay(1000);
		return true;
	}
	if(sw_parameter_get_int("dev_cert_install") == CHECK_SUCCESS) //判断是否成功安装过证书
	{
		struct stat file_stat;
		//确认对应的证书以及私钥是否存在
		if((lstat(DEVCERT,&file_stat) == 0) && (lstat(DEVCERTKEY,&file_stat) == 0))
		{
			dev_private_key_init();
			m_cer_status = true;
			return false;
		}
	}

	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

	bool install_result = false;
	EVP_PKEY *key = NULL;
	X509_REQ *req = NULL;
	X509 *mycert = NULL;
	key = EVP_PKEY_new();
	if(key == NULL)
	{
		HWIPTV_LOG_DEBUG("can not  to get rsa key");
		goto END;
	}
	if(!EVP_PKEY_assign_RSA(key,RSA_generate_key(RSA_KEY_BITS,RSA_F4,callback,NULL))) //获取密钥对
	{
		HWIPTV_LOG_DEBUG("faile to get EVP_PKEY_assign_RSA\n");
		goto END;
	}
	req = X509_REQ_new();
	if(req == NULL)
	{
		HWIPTV_LOG_DEBUG("fail to X509_REQ_new \n");
		goto END;
	}

	if(!set_dev_req_info(key,req))
	{
		HWIPTV_LOG_DEBUG("set_dev_req_info fail\n");
		goto END;
	}
	//X509_REQ_print_fp(stdout, req); //信息打印
	//PEM_write_X509_REQ(stdout, req); //写到标准输出

	mycert = X509_new();
	if(mycert == NULL )
	{
		HWIPTV_LOG_DEBUG("fail to  new X509 cert\n");
		goto END;
	}
	if(!sig_dev_by_middlecert(req,mycert,LIMITE_TIME,SERIAL_NUMBER))
	{
		HWIPTV_LOG_DEBUG("fail to sig_dev_by_middlecert \n");
		goto END;
	}
	if(!save_dev_private_key(key))
	{
		HWIPTV_LOG_DEBUG("fail to save_dev_private_key\n");
		goto END;
	}
	//X509_print_fp(stdout,mycert); //信息打印
	//PEM_write_X509(stdout,mycert);
	if(!save_dev_cert(mycert,DEVCERT)) //保存证书
	{
		HWIPTV_LOG_DEBUG("fail to  save cert to false\n");
		goto END;
	}
	install_result = is_install_x509_success();
	if(install_result)
	{
		//生成证书链,双向认证发送的是证书链 证书文件排序 设备证书 ===>中间证书 ==>根证书
		char cmd[128] = {0};
		sw_snprintf(cmd,sizeof(cmd),0,sizeof(cmd),"busybox cat %s >> %s",MIDLECERTCHAIN,DEVCERT);
		swsyscmd(cmd);
		dev_private_key_init();
		sw_parameter_set_int("dev_cert_install",1);
		sw_parameter_save();
		HWIPTV_LOG_DEBUG("x509 cert install sucess\n");
	}
	else
	{
		remove(DEVCERT);
		remove(DEVCERTKEY);
		sw_parameter_set_int("dev_cert_install",0);
		sw_parameter_save();
		HWIPTV_LOG_DEBUG("install X509 fail\n");
	}
	m_cer_status = install_result; //记录证书的最后状态

END:
	if(req)
	{
		X509_REQ_free(req);
		req = NULL;
	}
	if(mycert)
	{
		 X509_free(mycert);
		 mycert = NULL;
	}
	if(key)
	{
		EVP_PKEY_free(key);
		key = NULL;
	}
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	return false;
}

void hw_dev_cert_init(void)
{
	//先占用一个海思底层加密通道用资源
	swplatform_r2r_init();

	m_devcert_thrd = sw_thrd_open( "tMkdevcert", 155, 0, 64*1024, (threadhandler_t)x509_install_proc, 0, 0 );
	if(m_devcert_thrd)
		sw_thrd_resume( m_devcert_thrd);
	else
		HWIPTV_LOG_DEBUG("can not  open thrad tRcodertimeinfo\n");
	return;
}

void hw_dev_cert_deinit(void)
{
	//释放该通道一直占用的R2R解密通道资源
	swplatform_r2r_deinit();
}
