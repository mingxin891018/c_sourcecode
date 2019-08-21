#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <sys/types.h>
#include <unistd.h>

#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/ecerr.h>
#include <openssl/evp.h>

#define PUBLIC_KEY 	"./public_key.txt"
#define PRIVATE_KEY "./private_key.txt"
#define SIGNATURE_FILE "./sinature.txt"

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

static char m_str_rand_seed[] = "SUNNIWELL Beijing SBox upgrade sinature generate.";

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;

	if(!data || len <= 0){
		return;
	}

	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X", data[i]&0xff);
	printf("\n");
}

unsigned char *read_file(const char *path)
{
	int fd = -1;
	unsigned char *szBuf = NULL;
	size_t size = 0, re_read = 0;

	if(!path)
		goto ERR_EXIT;

	//读取private key
	fd = open(path, O_RDONLY);
	if(fd < 0){
		log_error("open %s failed!\n", path);
		goto ERR_EXIT;
	}

	size = lseek(fd, 0, SEEK_END);
	log_info("size=%zu\n", size);
	lseek(fd, 0, SEEK_SET);
	
	szBuf = (unsigned char *)malloc(size + 1);
	if(!szBuf){
		log_error("malloc szBuf failed!\n");
		goto ERR_EXIT;
	}
	memset(szBuf, 0, size + 1);
	while(size > 0){
		re_read = read(fd, szBuf + re_read, size);
		if(re_read < 0){
			log_error("read error ret=%zu\n", re_read);
			goto ERR_EXIT;
		}
		size -= re_read;
	}

ERR_EXIT:
	if(fd > 0)
		close(fd);

	return szBuf;
}

static int read_pub_point(const char *file_path, BIGNUM **big)
{
	unsigned char *s = NULL;
	unsigned char *file = read_file(PUBLIC_KEY);
	char *point1 = NULL, *point2 = NULL, *point3 = NULL;

	if(!file)
		return -1;
	log_info("file=%s\n", file);
	
	s = file;
	while(*s == ',' || *s == '"' || *s == '\n'){
		*s = '\0';
		s++;
	}
	
	point1 = s;
	while(*s != ',' && *s != '"' && *s != '\n' && *s != '\0') s++;
	while(*s == ',' || *s == '"' || *s == '\n'){
		*s = '\0';
		s++;
	}
	
	point2 = s;
	while(*s != ',' && *s != '"' && *s != '\n' && *s != '\0') s++;
	while(*s == ',' || *s == '"' || *s == '\n'){
		*s = '\0';
		s++;
	}
	
	point3 = s;
	while(*s != ',' && *s != '"' && *s != '\n' && *s != '\0') s++;
	while(*s == ',' || *s == '"' || *s == '\n'){
		*s = '\0';
		s++;
	}

	log_info("point1=%s\n", point1);
	log_info("point2=%s\n", point2);
	log_info("point3=%s\n", point3);
	
	big[0] = BN_bin2bn((const unsigned char *)point1, strlen(point1), NULL);
	big[1] = BN_bin2bn((const unsigned char *)point2, strlen(point2), NULL);
	big[2] = BN_bin2bn((const unsigned char *)point3, strlen(point3), NULL);
	
	if(!big[0] || !big[1] || !big[2] ){
		log_error("bignum error!\n");
		goto READ_END;
	}

READ_END:
	if(file)
		free(file);
	if(big[0])
		BN_free(big[0]);
	if(big[1])
		BN_free(big[1]);
	if(big[2])
		BN_free(big[2]);
	return 0;
}

int get_sha1_digest(const char *path, unsigned char *digest, int *dig_len)
{
	int ret = -1, fd = -1;
	size_t size = 0, re_read = 0;
	unsigned char *in_buf = NULL;
	
	if(!path || !digest || !dig_len)
		goto ERR_EXIT;
	
	EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
	if(!md_ctx){
		log_error("md_ctx is null!\n");
		goto ERR_EXIT;
	}

	EVP_DigestInit_ex(md_ctx,EVP_sha1(), NULL);

	//读取待签名的文件
	fd = open(path, O_RDWR);
	if(fd < 0)
		goto ERR_EXIT;
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	
	log_info("file %s length=%zu\n", path, size);
	in_buf = malloc(size);
	if(!in_buf)
		goto ERR_EXIT;

	while(size > 0){
		re_read = read(fd, in_buf, size);
		if(re_read < 0){
			log_error("read error ret=%zu\n", re_read);
			goto ERR_EXIT;
		}
		EVP_DigestUpdate(md_ctx, in_buf, re_read);
		size -= re_read;
	}

	EVP_DigestFinal_ex(md_ctx, digest, dig_len);
	ret = 0;

ERR_EXIT:
	if(md_ctx)
		EVP_MD_CTX_free(md_ctx);
	if(in_buf)
		free(in_buf);
	if(fd > 0)
		close(fd);

	return ret;
}

unsigned char *get_signature(const char *path)
{
	unsigned char *signature_buf = read_file(path);
	return signature_buf;
}

static int create_key(void)
{
	EC_KEY *eckey = NULL;
	EC_builtin_curve *curves = NULL;
	const BIGNUM *priv_key = NULL;
	const EC_POINT *pub_key = NULL;
	int ret = 0, nid = 0, crv_len = 0, i = 0;

	RAND_seed(m_str_rand_seed, sizeof(m_str_rand_seed));

	/* 获取实现的椭圆曲线个数 */
	crv_len = EC_get_builtin_curves(NULL, 0);
	curves = (EC_builtin_curve *)malloc(sizeof(EC_builtin_curve) * crv_len);

	/* 获取椭圆曲线列表 */
	//nid=curves[0].nid;会有错误，原因是密钥太短
	EC_get_builtin_curves(curves, crv_len);
	for(i = 0; i < crv_len; i++){
		log_info("curves[%d].comment=%s\n", i, curves[i].comment);
		log_info("curves[%d].nid=%d\n", i, curves[i].nid);
		if(curves[i].nid == 410){
			log_info("curves[%d]=410\n", i);
			break;
		}
	}
	nid=curves[i].nid;
	log_info("curves_num=%d,nid=%d\n", crv_len, nid);

	/* 根据选择的椭圆曲线生成密钥 */
	eckey = EC_KEY_new_by_curve_name(nid);
	if(eckey == NULL){
		log_error("create eckey err!\n");
		goto ERR_EXIT;
	}

	if (!EC_KEY_generate_key(eckey))
		goto ERR_EXIT;
	priv_key = EC_KEY_get0_private_key( eckey );
	pub_key = EC_KEY_get0_public_key( eckey );

	//保存private key
	{
		FILE* fp;
		char* fname = PRIVATE_KEY;
		char* priv_str = BN_bn2hex( priv_key );
		{
			fp = fopen(fname, "wb");
			if( fp )
			{
				fwrite( priv_str, 1, strlen(priv_str), fp );
				log_info("set priv=%s success!\n", priv_str);
				fclose(fp);
			}
		}
		OPENSSL_free( priv_str );
	}
	//保存public key
	{
		FILE* fp;
		char* fname = PUBLIC_KEY;
		char* pub_str;
		fp = fopen(fname, "w+");
		if( fp )
		{
			BIGNUM *x, *y, *z;
			x = BN_new();
			y = BN_new();
			z = BN_new();
			if( EC_POINT_get_Jprojective_coordinates_GFp( EC_KEY_get0_group(eckey), pub_key, x, y, z, NULL ) )
			{
				pub_str = BN_bn2hex(x);
				fprintf( fp, "\"%s\",\n", pub_str );
				OPENSSL_free(pub_str);
				pub_str = BN_bn2hex(y);
				fprintf( fp, "\"%s\",\n", pub_str );
				OPENSSL_free(pub_str);
				pub_str = BN_bn2hex(z);
				fprintf( fp, "\"%s\"\n", pub_str );
				OPENSSL_free(pub_str);
				log_info("set public_key to group success!\n");
			}
			BN_free( x );
			BN_free( y );
			BN_free( z );
			fclose(fp);
		}
	}
	ret = 1;

ERR_EXIT:
	if( eckey )
		EC_KEY_free( eckey );
	if(curves)
		free(curves);
	return ret;
}

//输出签名结果
static int create_signature( const char *path)
{
	unsigned char digest[32] = {0};
	unsigned char *szBuf = NULL, *out_buf = NULL;
	int ret = -1, crv_len = 0, nid = 0, i = 0, dig_len = 0, sign_len = 0;

	EC_KEY *eckey = NULL;
	BIGNUM *priv_key = NULL;
	EC_POINT *pub_key = NULL;
	EC_builtin_curve *curves = NULL;

	//获取签名文件的摘要信息
	if(get_sha1_digest(path, digest, &dig_len) != 0){
		log_error("sha1 digest failed!\n");
		goto ERR_EXIT;
	}
	log_info("sha1 digest data:\n");
	bin_print(digest, dig_len);

	//读取private key
	szBuf = read_file(PRIVATE_KEY);
	if(!szBuf){
		log_error("read public_key error!\n");
		goto ERR_EXIT;
	}
	log_info("priv_key=%s\n", szBuf);
	priv_key = BN_bin2bn((const unsigned char *)szBuf, strlen(szBuf), NULL);
	if(!priv_key){
		log_error("read priv_key failed!\n");
		goto ERR_EXIT;
	}

	/* 获取实现的椭圆曲线个数 */
	crv_len = EC_get_builtin_curves(NULL, 0);
	curves = (EC_builtin_curve *)malloc(sizeof(EC_builtin_curve) * crv_len);

	/* 获取椭圆曲线列表 */
	//nid=curves[0].nid;会有错误，原因是密钥太短
	EC_get_builtin_curves(curves, crv_len);
	for(i = 0; i < crv_len; i++){
		if(curves[i].nid == 410){
			log_info("curves[%d]=410\n", i);
			break;
		}
	}
	nid=curves[i].nid;
	log_info("curves_num=%d,nid=%d\n", crv_len, nid);

	eckey = EC_KEY_new_by_curve_name(nid);
	if(eckey == NULL){
		log_error("create eckey err!\n");
		goto ERR_EXIT;
	}

	/* 为产生的eckey设置私钥priv_key 和公钥 pub_key */
	if(EC_KEY_set_private_key( eckey, priv_key ) != 1){
		log_error("set private_key success!\n");
		goto ERR_EXIT;
	}
	
	/* 根据选择的椭圆曲线和私钥生成公钥 */
	//pub_key = EC_POINT_new(EC_KEY_get0_group(eckey));
	//if(EC_POINT_mul(EC_KEY_get0_group(eckey), pub_key, priv_key, NULL, NULL, NULL) != 1){
	//	log_error("set pub_key point failed!\n");
	//	goto ERR_EXIT;
	//}

	//if(EC_KEY_set_public_key(eckey, pub_key) != 1){
	//	log_error("set private_key success!\n");
	//	goto ERR_EXIT;
	//}

	//检测eckey project是否正确
	if(EC_KEY_check_key(eckey) != 1){
		log_error("check eckey failed!\n");
		goto ERR_EXIT;
	}
	log_info("check eckey success!\n");
	
	//签名
	sign_len = ECDSA_size(eckey);
	out_buf = malloc(sign_len);
	if(!out_buf){
		log_error("malloc filed!\n");
		goto ERR_EXIT;
	}
	unsigned char *pp = out_buf;
	memset(out_buf, 0, sign_len);
	log_info("sign_len=%d\n", sign_len);
	if(ECDSA_sign(0, (const unsigned char *)digest, dig_len, pp, &sign_len, eckey) != 1){
		log_error("ecdsa sign failed!\n");
		goto ERR_EXIT;
	}

	log_info("signature success!\n");
	bin_print(out_buf, sign_len);

	//签名数据存盘
	FILE *file = fopen(SIGNATURE_FILE, "w");
	if(!file)
		goto ERR_EXIT;
	for(i = 0; i < sign_len; i++)
		fprintf(file, "%02X", out_buf[i]);
	fclose(file);
	ret = 1;

ERR_EXIT:
	if(szBuf)
		free(szBuf);
	
	if(curves)
		free(curves);
	if( eckey )
		EC_KEY_free( eckey );
	if(pub_key)
		EC_POINT_free(pub_key);
	if( priv_key )
		BN_free( priv_key );

	return ret;
}

//验证签名结果
int check_signature(const char *file_path)
{
	unsigned int  dig_len = 0;
	unsigned char digest[32] = {0};
	EC_builtin_curve *curves = NULL;
	int ret = 0, crv_len = 0, nid = 0, i = 0;

	EC_POINT *pub_key = NULL;
	EC_KEY *eckey = NULL;
	unsigned char *signature = NULL;

	/* 获取实现的椭圆曲线个数 */
	crv_len = EC_get_builtin_curves(NULL, 0);
	curves = (EC_builtin_curve *)malloc(sizeof(EC_builtin_curve) * crv_len);

	/* 获取椭圆曲线列表 */
	//nid=curves[0].nid;会有错误，原因是密钥太短
	EC_get_builtin_curves(curves, crv_len);
	for(i = 0; i < crv_len; i++){
		if(curves[i].nid == 410){
			log_info("curves[%d]=410\n", i);
			break;
		}
	}
	nid=curves[i].nid;
	log_info("curves_num=%d,nid=%d\n", crv_len, nid);

	/* 根据选择的椭圆曲线生成密钥 */
	eckey = EC_KEY_new_by_curve_name(nid);
	if(eckey == NULL){
		log_error("create eckey err!\n");
		goto ERR_EXIT;
	}
	/* 读取公钥坐标点  */
	BIGNUM *pub_point[3];
	if(read_pub_point(PUBLIC_KEY, pub_point) != 0)
		goto ERR_EXIT;

	/* 生成公钥坐标点  */
	pub_key = EC_POINT_new( EC_KEY_get0_group(eckey));
	if(!pub_key){
		log_error("create public_key error!\n");
		goto ERR_EXIT;
	}

	EC_POINT_set_Jprojective_coordinates_GFp( EC_KEY_get0_group(eckey), pub_key, pub_point[0], pub_point[1], pub_point[2], NULL );
	EC_KEY_set_public_key(eckey, pub_key );

	//获取签名文件的摘要信息
	get_sha1_digest(file_path, digest, &dig_len);
	log_info("sha1 digest:\n");
	bin_print(digest, dig_len);

	signature = get_signature(SIGNATURE_FILE);
	if(!signature){
		log_info("get signature failed!\n");
		goto ERR_EXIT;
	}

	if (ECDSA_verify(0, digest, 20, signature, strlen(signature), eckey) != 1)
	{
		log_error("verify error\n");
		goto ERR_EXIT;
	}
	log_error("verify success\n");
	ret = 0;

ERR_EXIT:
	if( eckey )
		EC_KEY_free( eckey );
	
	return ret;
}

void showUsage(void)
{
	printf("Options:			\n" );
	printf(" --create_key		\n" );
	printf(" --create_sign=FILE	\n"	);
	printf(" --check_sign=FILE	\n"	);
	printf(" --ecdsa_test		\n"	);
	printf(" --version			\n"	);
	printf(" --help				\n"	);
}

void showVersion(void)
{
	printf("ECDSA_SIG_TOOLS version=1.00\n");
}

int main(int argc, char* argv[])
{
	int c;

	while(1) {
		static struct option longOpts[] = {
			{ "create_key", 	no_argument, 		NULL,	'k'		},
			{ "create_sign", 	required_argument, 	NULL,	's'		},
			{ "check_sign", 	required_argument, 	NULL,	'g'		},
			{ "ecdsa_test", 	no_argument, 		NULL,	't'		},
			{ "version", 		no_argument,		NULL, 	'v' 	},
			{ "help", 			no_argument, 		NULL, 	'h' 	},
			{ NULL, 0, NULL, 0},
		};

		c = getopt_long(argc, argv, "ksg:vh", longOpts, NULL);
		if(c == -1) {
			break;
		}
		switch(c) {
			case 'k':
				   create_key();
				   break;
			case 's':
				   printf("optarg=%s\n", optarg);
				   create_signature(optarg);
				   break;
			case 'g':
				   printf("input file: %s\n",optarg);
				   check_signature(optarg);
				   break;
			case 'v':
				   showVersion();
				   exit(0);
			case 't':
				   ecdsa_test();
				   exit(0);
			case 'h':
				   showUsage();
				   exit(0);
			default:
				   showUsage();
				   exit(1);
		}
	}
	return 0;
}

int ecdsa_test(void)
{
	EC_KEY *key1,*key2;
	const EC_POINT *pubkey1,*pubkey2;
	EC_GROUP *group1,*group2;
	unsigned int ret = 0, nid = 0, size = 0, i = 0, sig_len = 0;
	unsigned char *signature,digest[20];
	EC_builtin_curve *curves;
	int crv_len;
	char shareKey1[128],shareKey2[128];
	int len1,len2;


	/* 构造 EC_KEY 数据结构 */
	key1=EC_KEY_new();
	if(key1==NULL)
	{
		printf("EC_KEY_new err!\n");
		return -1;
	}
	key2=EC_KEY_new();
	if(key2==NULL)
	{
		printf("EC_KEY_new err!\n");
		return -1;
	}
	/* 获取实现的椭圆曲线个数 */
	crv_len = EC_get_builtin_curves(NULL, 0);
	curves = (EC_builtin_curve *)malloc(sizeof(EC_builtin_curve) * crv_len);

	/* 获取椭圆曲线列表 */
	//nid=curves[0].nid;会有错误，原因是密钥太短
	EC_get_builtin_curves(curves, crv_len);
	nid=curves[25].nid;

	/* 根据选择的椭圆曲线生成密钥参数 group */
	group1=EC_GROUP_new_by_curve_name(nid);
	if(group1==NULL)
	{
		printf("EC_GROUP_new_by_curve_name err!\n");
		return -1;
	}
	group2=EC_GROUP_new_by_curve_name(nid);
	if(group1==NULL)
	{
		printf("EC_GROUP_new_by_curve_name err!\n");
		return -1;
	}

	/* 设置密钥参数 */
	ret=EC_KEY_set_group(key1,group1);
	if(ret!=1)
	{
		printf("EC_KEY_set_group err.\n");
		return -1;
	}
	ret=EC_KEY_set_group(key2,group2);
	if(ret!=1)
	{
		printf("EC_KEY_set_group err.\n");
		return -1;
	}

	/* 生成密钥 */
	ret=EC_KEY_generate_key(key1);
	if(ret!=1)
	{
		printf("EC_KEY_generate_key err.\n");
		return -1;
	}
	ret=EC_KEY_generate_key(key2);
	if(ret!=1)
	{
		printf("EC_KEY_generate_key err.\n");
		return -1;
	}

	/* 检查密钥 */
	ret=EC_KEY_check_key(key1);
	if(ret!=1)
	{
		printf("check key err.\n");
		return -1;
	}

	/* 获取密钥大小 */
	size=ECDSA_size(key1);
	printf("size %d \n",size);
	for(i=0;i<20;i++)
		memset(&digest[i],i+1,1);
	signature= (unsigned char*)malloc(size);

	/* 签名数据，本例未做摘要，可将 digest 中的数据看作是 sha1 摘要结果 */
	ret=ECDSA_sign(0,digest,20,signature,&sig_len,key1);
	if(ret!=1)
	{
		printf("sign err!\n");
		return -1;
	}
	/* 验证签名 */
	ret=ECDSA_verify(0,digest,20,signature,sig_len,key1);
	if(ret!=1)
	{
		printf("ECDSA_verify err!\n");
		return -1;
	}
	/* 获取对方公钥，不能直接引用 */
	pubkey2 = EC_KEY_get0_public_key(key2);
	/* 生成一方的共享密钥 */
	len1= ECDH_compute_key(shareKey1, 128, pubkey2, key1, NULL);
	pubkey1 = EC_KEY_get0_public_key(key1);
	/* 生成另一方共享密钥 */
	len2= ECDH_compute_key(shareKey2, 128, pubkey1, key2, NULL);
	if(len1!=len2)
	{
		printf("err\n");
	}
	else
	{
		ret=memcmp(shareKey1,shareKey2,len1);
		if(ret==0)
			printf("生成共享密钥成功\n");
		else
			printf("生成共享密钥失败\n");
	}
	printf("test ok!\n");
	EC_KEY_free(key1);
	EC_KEY_free(key2);
	free(signature);
	free(curves);
	return 0;
}


