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
#define SIGNATURE_FILE "sinature.txt"

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

//生成公钥和私钥

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;
	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X", data[i]&0xff);
	printf("\n");
}

static int create_key(const char *path)
{
	EC_KEY *key1,*key2;
	const EC_POINT *pubkey1,*pubkey2;
	EC_GROUP *group1,*group2;
	unsigned int ret,nid,size,i,sig_len;
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
	bin_print((const unsigned char *)shareKey1, len1);
	pubkey1 = EC_KEY_get0_public_key(key1);
	
	/* 生成另一方共享密钥 */
	len2= ECDH_compute_key(shareKey2, 128, pubkey1, key2, NULL);
	bin_print((const unsigned char *)shareKey2, len2);
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

//输出签名结果
static int create_signature( const char *path)
{
	return 0;
}

//验证签名结果
int check_signature(const char *file_path)
{

	return 0;
}

void showUsage(void)
{
	printf("Options:			\n" );
	printf(" --create_key		\n" );
	printf(" --create_sign=FILE	\n"	);
	printf(" --check_sign=FILE	\n"	);
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
				   create_key(optarg);
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


