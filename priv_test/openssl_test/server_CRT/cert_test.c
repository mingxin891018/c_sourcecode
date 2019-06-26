/**
 * @file cert_openssl.c
 * @brief 利用openssl api处理证书
 * @author zy
 * @date 2014-10-11 modify
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/pkcs12.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>

#define CONTEXT_MAX_NUM 11
#define SERIAL_RAND_BITS 64
#define CA_FILE "../CA/sign.crt"
#define CA_KEY "../CA/ssl.key"
#define USR_KEY "./cert_pem.key"
#define USR_CERT "./cert_pem.crt"

/**
 * 描  述: 获取X509对象
 * 参  数: @param[IN] cert_file  证书
 * 返回值: X509对象
 */
X509* read_public_cert(const char* cert_file)
{
	X509 *x509 = NULL;

	FILE *fp = fopen (cert_file, "r");   
	if(!fp)
	{
		printf("read_public_cert, open cert failed!");
		return NULL; 
	}
	x509 = PEM_read_X509(fp, NULL, 0, NULL);
	if(x509 == NULL) 
	{  
		printf("read_public_cert, get x509 failed!");
		return NULL;   
	}
	return x509;
}
/**
 * 描  述: 获取公钥
 * 参  数: @param[IN] cert_file  证书
 * 返回值: 公钥
 */
EVP_PKEY * read_public_key(const char* cert_file)
{
	X509 *x509 = NULL;
	EVP_PKEY *pkey = NULL;
	FILE *fp = fopen (cert_file, "r");   
	if(!fp)
	{
		printf("read_public_key, open cert failed!");
		return NULL;
	}
	x509 = PEM_read_X509(fp, NULL, 0, NULL);
	if(x509 == NULL) 
	{  
		printf("read_public_key, get x509 failed!");
		return NULL;   
	}
	fclose(fp);
	pkey = X509_extract_key(x509);
	X509_free(x509);
	if(pkey == NULL)
	{
		printf("read_public_key, get key failed!");
	}
	return pkey; 
}
/**
 * 描  述: 获取私钥
 * 参  数: @param[IN] key_file  证书
 * 返回值: 私钥
 */
EVP_PKEY *read_private_key(const char* key_file)
{
	EVP_PKEY *pkey = NULL;

	FILE *fp = fopen(key_file, "r");
	if(!fp)
	{
		printf("read_private_key, open key failed!");
		return NULL;
	}
	pkey = PEM_read_PrivateKey(fp, NULL, 0, NULL);
	fclose(fp);
	if (pkey == NULL)
	{
		printf("read_private_key, get key failed!");
	}
	return pkey;
}
/**
 * 描  述: 添加证书内容
 * 参  数: @param[IN] name  X509_NAME
 @param[IN] ctx   使用者信息
 @param[IN] num   ctx数组长度
 * 返回值: 1: 成功 0: 失败
 */
int add_cert_ctx(X509_NAME* name, char* ctx[], int num)
{
	int i = 0;
	int max = 0;
	//int item[] = {NID_commonName, NID_countryName, NID_stateOrProvinceName, NID_localityName, NID_organizationName, NID_organizationalUnitName, NID_pkcs9_emailAddress};
	int item[] = {	NID_commonName,          		NID_countryName,               NID_stateOrProvinceName, 
					NID_localityName,             	NID_organizationName,          NID_organizationalUnitName, 
					NID_pkcs9_emailAddress,   		NID_basic_constraints,         NID_netscape_comment,
					NID_subject_key_identifier, 	NID_authority_key_identifier 
				};
	max = sizeof(item)/sizeof(item[0]);
	max = max > num ? num : max;
	for(i=0; i<max; ++i)
	{
		if(!X509_NAME_add_entry_by_NID(name, item[i], MBSTRING_UTF8, ctx[i], -1, -1, 0))
		{
			printf("add_cert_ctx, add entry:%d to %s failed!", item[i], ctx[i]);
			return 0;
		}
	}
	return 1;
}

/**
 * 描  述: 创建证书密钥
 * 参  数: @param[OUT] pkey  EVP_PKEY
 @param[IN] bits   密钥长度
 * 返回值: 1: 成功 0: 失败
 */
static RSA *rsa = NULL;
int create_client_key(EVP_PKEY** pkey, int bits)
{
	int ret = 0;
	FILE *fp = NULL;
	EVP_PKEY *pk = NULL;
	if(NULL == (rsa = RSA_generate_key(bits, RSA_F4, NULL, NULL)))
		goto err;
	if(NULL == (fp = fopen(USR_KEY, "w")))
	{
		printf("fopen %s failed\n",USR_KEY);
		goto err;
	}
	printf("RSA_generate_key ok\n");
	PEM_write_RSAPrivateKey(fp, rsa, NULL, NULL, 0, NULL, NULL);

	if((pk = EVP_PKEY_new()) == NULL)
	{
		printf("create_client_key, gen new key failed!");
		goto err;
	}
	if(!EVP_PKEY_assign_RSA(pk, rsa))
	{
		printf("create_client_key, assign key failed!");
		goto err;
	}
	printf("EVP_PKEY_assign_RSA ok\n");
	*pkey = pk;
	ret = 1;

err:
	if(fp){
		fclose(fp);
		fp = NULL;
	}
	return ret;
}

/**
 * 描  述: CA签发证书
 * 参  数: @param[OUT] x509p      EVP_PKEY
 @param[OUT] pkey       X509
 @param[IN] ca_file      CA
 @param[IN] ca_key_file   CA密钥
 @param[IN] serial      序列号
 @param[IN] days        过期时长
 * 返回值: 1: 成功 0: 失败
 */
int create_ca_signed_crt(X509** x509p, EVP_PKEY** pkey, const char* ca_file, const char* ca_key_file, const char* user, const int serial, const int days)
{
	X509* x = NULL;
	EVP_PKEY* pk = NULL;
	X509* xca = NULL;
	EVP_PKEY* xca_key = NULL;
	X509_NAME* name = NULL;
	//X509_NAME *tname = NULL;
	//char buf[256] = {0};
	
	//初始化证书信息
	//char* ctx[] = {(char*)user, "bb", "cc", "dd", "ee", "ff", "ff@sf.com"};
	char *ctx[] = {"EC2108v8", "CN", "GD", "SZ", "Huawei", "STB PDU", "zhaomingxin@sunniwell.com", "CA:FALSE", "OpenSSL EC2108CV5 Certificate", "hash", "keyid:always"};

	//生成用户证书的RSA密钥对
	if(!create_client_key(&pk, 2048))
	{
		printf("create_ca_signed_crt, gen key failed!");
		goto err;
	}
	if((x = X509_new()) == NULL)
	{
		printf("create_ca_signed_crt, gen x509 failed!");
		goto err;
	}
	
	//读取CA证书及密钥并验证密钥
	xca = read_public_cert(ca_file);
	xca_key = read_private_key(ca_key_file);
	if(!X509_check_private_key(xca, xca_key))
	{
		printf("create_ca_signed_crt, check ca %s and key %s failed!", ca_file, ca_key_file);
		goto err;
	}

	//设置被签名的CA机构
	if(!X509_set_issuer_name(x, X509_get_subject_name(xca)))
	{
		printf("create_ca_signed_crt, set issuer failed!");
		goto err;
	}

	//设置证书版本号
	ASN1_INTEGER_set(X509_get_serialNumber(x), serial);
	
	//设置证书有效期
	if(X509_gmtime_adj(X509_get_notBefore(x), 0L) == NULL)
	{
		printf("create_ca_signed_crt, set cert begin time failed!");
		goto err;
	}
	if(X509_gmtime_adj(X509_get_notAfter(x), (long)60*60*24*days) == NULL)
	{
		printf("create_ca_signed_crt, set cert expired time failed!");
		goto err;
	}

	//设置证书使用的公钥
	if(!X509_set_pubkey(x, pk))
	{
		printf("create_ca_signed_crt, set pubkey failed!");
		goto err;
	}


	//设置证书的信息
	//tname = X509_get_subject_name(xca);
	//X509_NAME_get_text_by_NID(tname, NID_localityName, buf, 256);
	//printf("city: %s\n", buf);
	name = X509_get_subject_name(x);
	if(!add_cert_ctx(name, ctx, CONTEXT_MAX_NUM))
	{
		printf("create_ca_signed_crt, add entry failed!");
		goto err;
	}
	
	//用CA证书给用户证书签名
	if(!X509_sign(x, xca_key, EVP_sha1()))
	{
		printf("create_ca_signed_crt, sign cert failed!");
		goto err;
	}
	*pkey = pk;
	*x509p = x;
	return 1;
err:
	if(x)
		X509_free(x);
	if(pk)
		EVP_PKEY_free(pk);
	if(rsa)
		RSA_free(rsa);
	return 0;
}

/**
 * 描  述: 创建P12证书
 * 参  数: @param[IN] p12_file     p12
 @param[IN] p12_passwd   p12密码
 @param[IN] ca_file      CA
 @param[IN] ca_key_file   CA密钥
 @param[IN] serial      序列号
 @param[IN] days        过期时长
 * 返回值: 1: 成功 0: 失败
 */
int create_p12_cert(char* p12_file, char* p12_passwd, const char* ca_file, const char* ca_key_file, const char* user, const int serial, const int days)
{
	int ret = 0;
	PKCS12* p12 = NULL;
	X509* cert = NULL;
	EVP_PKEY* pkey = NULL;
	FILE *fp = NULL;
	BIO *mem = NULL;
	char *mem_out = NULL;
	long len = 0;
	
	printf("create_p12_cert begin\n");
	//加载SSL相关算法
	//SSLeay_add_all_algorithms();
	OpenSSL_add_all_algorithms();

	printf("SSLeay_add_all_algorithms ok\n");
	//生成CA签名过的用户证书X509
	if(!create_ca_signed_crt(&cert, &pkey, ca_file, ca_key_file, user, serial, days))
	{
		printf("create_p12_cert, create signed cert failed!");
		goto err;
	}
	
	printf("create_ca_signed_crt ok");
#if 1	
	//将证书保存成pem格式
	fp = fopen(p12_file, "wb");	
	PEM_write_X509(fp,cert);
	fclose(fp);
	fp = NULL;
	
	printf("PEM_read_X509 ok\n");
#else
	//将X509证书转换成p12格式证书PEM_write_X509
	p12 = PKCS12_create(p12_passwd, NULL, pkey, cert, NULL, 0,0,0,0,0);
	if(!p12)
	{
		printf("create_p12_cert, create p12 object failed!");
		goto err;
	}
	/*
	   fp = fopen(p12_file, "wb");
	   if(!fp)
	   {
	   printf("create_p12_cert, open/create p12 file failed!");
	   goto err;
	   }
	   if(!i2d_PKCS12_fp(fp, p12))
	   {
	   printf("create_p12_cert, put p12 file failed!");
	   goto err;
	   }
	 */

	//存盘p12证书
	fp = fopen(p12_file, "wb");
	if(!fp)
	{
		printf("create_p12_cert, open/create p12 file failed!");
		goto err;
	}

	mem = BIO_new(BIO_s_mem());
	if(!mem)
	{
		printf("create_p12_cert, BIO_new failed!");
		goto err;
	}

	i2d_PKCS12_bio(mem, p12);
	len = BIO_get_mem_data(mem, &mem_out);
	fwrite(mem_out, sizeof(char), len, fp);
	ret = 1;
#endif

err:
	if(cert)
		X509_free(cert);
	if(pkey)
		EVP_PKEY_free(pkey);
	if (p12)
		PKCS12_free(p12);
	if (mem)
		BIO_free(mem);
	if(fp)
		fclose(fp);

	EVP_cleanup();
	return ret;
}

//openssl smime -sign -in unsign.mc -out signed.mc -signer ssl.crt -inkey ssl.key -certfile server.crt -outform der -nodetach
int sign_mobile_config(const char *inmc, int inlen, char **outmc, int *outlen, char *certfile, char *signerfile, char *keyfile)
{
	X509 *signer = NULL;
	EVP_PKEY *key = NULL;
	PKCS7 *p7 = NULL;
	X509 *cert = NULL;
	BIO *in = NULL;
	BIO *out = NULL;
	STACK_OF(X509) * other = NULL;

	int mclen = 0;
	char *mc = NULL;
	char *tmp = NULL;
	OpenSSL_add_all_algorithms();
	//SSLeay_add_all_algorithms();

	// in 
	in = BIO_new_mem_buf((char*)inmc, inlen); // BIO_write
	if (!in)
	{
		return -1;
	}
	cert = read_public_cert(certfile);

	other = sk_X509_new_null();
	sk_X509_push(other, cert);

	signer = read_public_cert(signerfile);
	key = read_private_key(keyfile);
	if (!X509_check_private_key(signer, key))
	{
		printf("x509 check failed!\n");
		return -1;
	}

	p7 = PKCS7_sign(signer, key, other, in, 0);
	if (!p7)
		goto end;
	// out
	out = BIO_new(BIO_s_mem());
	i2d_PKCS7_bio(out, p7);
	mclen = BIO_get_mem_data(out, &tmp);// BIO_read
	mc = (char*)malloc(mclen);
	memcpy(mc, tmp, mclen);
	*outmc = mc;
	*outlen = mclen;

end:
	sk_X509_pop_free(other, X509_free);
	X509_free(signer);
	EVP_PKEY_free(key);
	PKCS7_free(p7);
	BIO_free(in);
	BIO_free_all(out);
	return 0;
}

int main()
{
	create_p12_cert(USR_CERT, "1234", CA_FILE, CA_KEY, "mingxin1", 3, 3650);
	return 0;
}

