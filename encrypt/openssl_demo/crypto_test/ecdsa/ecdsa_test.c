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
static char m_str_rand_seed[] = "SUNNIWELL Beijing SBox upgrade sinature generate.";

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
	int ret = 0;
	EC_KEY *eckey = NULL;
	const BIGNUM *priv_key = NULL;
	const EC_POINT *pub_key = NULL;

	RAND_seed(m_str_rand_seed, sizeof(m_str_rand_seed));

	/* create the key */
	if ((eckey = EC_KEY_new_by_curve_name(410)) == NULL)
		goto ERR_EXIT;
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
		fp = fopen(fname, "wb");
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
	return ret;
}

#if 1
//输出签名结果
static int create_signature( const char *path)
{
	int fd = -1, ret = 0;
	unsigned char digest[64];
	unsigned char out_buf[64];
	unsigned int  dgst_len = 0;
	char *szBuf = NULL, *in_buf = NULL;
	size_t size = 0, re_read = 0;
	
	BIGNUM *priv_key = NULL;
	EC_KEY *eckey = NULL;
	EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
	ECDSA_SIG *signature = NULL;

	if(!md_ctx){
		log_error("md_ctx is null!\n");
		goto ERR_EXIT;
	}

	//read private_key
	fd = open(PRIVATE_KEY, O_RDWR);
	if(fd < 0){
		log_error("open %s failed!\n", PRIVATE_KEY);
		goto ERR_EXIT;
	}
	
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	szBuf = malloc(size);
	if(!szBuf){
		log_error("malloc szBuf failed!\n");
		goto ERR_EXIT;
	}

	log_info("priv_key size=%zu\n", size);
	while(size > 0){
		re_read = read(fd, szBuf + re_read, size);
		if(re_read < 0){
			log_error("read error ret=%zu\n", re_read);
			goto ERR_EXIT;
		}
		size -= re_read;
		log_info("re_read=%zu\n", re_read);
	}
	close(fd);
	fd = -1;

	printf("use ecc key **************%s\n", szBuf + 18);
	if( !BN_hex2bn( &priv_key, szBuf ) )
		goto ERR_EXIT;
	if ((eckey = EC_KEY_new_by_curve_name(410)) == NULL)
		goto ERR_EXIT;
	
	EC_KEY_set_private_key( eckey, priv_key );
	EVP_DigestInit_ex(md_ctx,EVP_sha256(), NULL);

	//读取待签名的文件
	fd = open(path, O_RDWR);
	if(fd < 0)
		goto ERR_EXIT;
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

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
		log_info("re_read=%zu\n", re_read);
	}

	//签名，信息存盘
	EVP_DigestFinal_ex(md_ctx, digest, &dgst_len);
	bin_print((const unsigned char *)digest, dgst_len);

	signature = ECDSA_do_sign(digest, dgst_len, eckey);
	if(signature == NULL)
		goto ERR_EXIT;

	memset(out_buf, 0, sizeof(out_buf));
	BN_bn2bin( signature->r, out_buf );
	BN_bn2bin( signature->s, out_buf+24 );

	bin_print((const unsigned char *)signature, 24);
	FILE *file = fopen(SIGNATURE_FILE, "w");
	if(!file)
		goto ERR_EXIT;
	fprintf(file, "%s", out_buf);
	fclose(file);
	ret = 1;

ERR_EXIT:
	if(szBuf)
		free(szBuf);
	if(in_buf)
		free(in_buf);
	if(fd > 0)
		close(fd);
	if( eckey )
		EC_KEY_free( eckey );
	if( priv_key )
		BN_free( priv_key );
	if( signature )
		ECDSA_SIG_free( signature );
	if(md_ctx)
		EVP_MD_CTX_free(md_ctx);
	return ret;
}

#endif
#if 0
//验证签名结果
typedef struct signature_context
{
	EC_POINT *pub_key;
	EC_KEY *eckey;
	ECDSA_SIG *signature;
	EVP_MD_CTX md_ctx;
	char    dummy[32];    /*此数据为无效数据，防止不同的openssl版本导致的EVP_MD_CTX结构体大小不一引起的死机问题*/
	unsigned char buf[650];
	int buflen;
	bool bupdate;
}sw_sig_context;

HANDLE sw_signature_create(const char *pubkey[], int nid, uint8_t* signature_buf)
{
	if (sw_parameter_get_int("upgrade_signature") == 0)
		return NULL;
	if ( pubkey == NULL )
		return NULL;
	sw_sig_context *context = (sw_sig_context*)malloc(sizeof(sw_sig_context));
	if ( context == NULL )
		return NULL;
	BIGNUM *pub[3];
	int i;
	memset( pub, 0, sizeof(pub) );
	memset(context, 0, sizeof(sw_sig_context));
	if ((context->eckey = EC_KEY_new_by_curve_name(410)) == NULL)
		goto ERROR_CREATE;
	if ( pubkey != NULL )
	{        
		for( i=0; i<3; i++ )
		{
			BN_hex2bn( pub+i, pubkey[i] );
			if (pub[i] == NULL)
				goto ERROR_CREATE;
		}
		if ((context->pub_key = EC_POINT_new( EC_KEY_get0_group(context->eckey) )) == NULL )
			goto ERROR_CREATE;
		EC_POINT_set_Jprojective_coordinates_GFp( EC_KEY_get0_group(context->eckey), context->pub_key, pub[0], pub[1], pub[2], NULL );
		EC_KEY_set_public_key( context->eckey, context->pub_key );
	}
	if ( signature_buf != NULL )
	{
		/* create the signature */
		if ((context->signature = ECDSA_SIG_new()) == NULL)
			goto ERROR_CREATE;
		BN_bin2bn( (unsigned char*)signature_buf, 24, context->signature->r );
		BN_bin2bn( (unsigned char*)signature_buf+24, 24, context->signature->s );
	}
	EVP_MD_CTX_init(&context->md_ctx);
	EVP_DigestInit(&context->md_ctx, EVP_ecdsa());
	context->buflen = 0;
	context->bupdate = false;
	return context;
ERROR_CREATE:
	if( context->eckey )
		EC_KEY_free( context->eckey );
	if( context->pub_key )
		EC_POINT_free( context->pub_key );
	if( context->signature )
		ECDSA_SIG_free( context->signature );
	for( i=0; i<3; i++ )
	{    
		if ( pub[i] )
			BN_free( pub[i] );
	}
	free(context);
	SWUPG_LOG_INFO("%s fail\n", __FUNCTION__);
	return NULL;
}

bool sw_signature_update(HANDLE scontex, uint8_t *inbuf, int inlen)
{
	if (inbuf == NULL || inlen <= 0)
		return false;
	unsigned char *in_buf = inbuf;
	int in_len = inlen;
	sw_sig_context *context = (sw_sig_context *)scontex;
	if ( context == NULL )
		return false;
	int tlen = in_len + context->buflen;
	context->bupdate = true;
	if ( tlen < 640 )
	{
		memcpy(&(context->buf[context->buflen]), in_buf, in_len);
		context->buflen = tlen;
		return true;
	}
	if ( context->buflen != 0 )
	{
		memcpy(&(context->buf[context->buflen]), in_buf, 640-context->buflen);
		EVP_DigestUpdate(&context->md_ctx, &context->buf, 64);
		in_len = in_len - (640-context->buflen);
		in_buf = &in_buf[640-context->buflen];
		context->buflen = 0;
	}
	while ( 640 <= in_len )
	{
		EVP_DigestUpdate(&context->md_ctx, in_buf, 64);
		in_len = in_len - 640;
		in_buf = &in_buf[640];
	}
	if (0 < in_len)
	{
		memcpy(&context->buf, in_buf, in_len);
		context->buflen = in_len;
	}
	return true;
}

bool sw_signature_final(HANDLE scontex, uint8_t *signature_buf)
{
	if (sw_parameter_get_int("upgrade_signature") == 0)
		return true;
	unsigned char digest[64];
	unsigned int  dgst_len = 0;
	bool ret = false;
	sw_sig_context *context = (sw_sig_context*)scontex;
	if ( context == NULL )
	{
		SWUPG_LOG_ERROR("%s handle is null\n", __FUNCTION__);
		return false;
	}
	if ( !context->bupdate )
	{
		SWUPG_LOG_INFO("not update\n");
		ret = true;
		goto END_VERIFY;
	}    
	if ( context->buflen != 0 )
	{
		EVP_DigestUpdate(&context->md_ctx, context->buf, context->buflen < 64 ? context->buflen : 64);
	}
	EVP_DigestFinal(&context->md_ctx, digest, &dgst_len);
	/* create the signature */
	if ( signature_buf )
	{
		if( context->signature )
			ECDSA_SIG_free( context->signature );
		if ((context->signature = ECDSA_SIG_new()) == NULL)
			goto END_VERIFY;
		BN_bin2bn( (unsigned char*)signature_buf, 24, context->signature->r );
		BN_bin2bn( (unsigned char*)signature_buf+24, 24, context->signature->s );
	}
	if (ECDSA_do_verify(digest, 20, context->signature, context->eckey) != 1)
	{
		SWUPG_LOG_INFO("verify error\n");
		goto END_VERIFY;
	}
	ret = true;
END_VERIFY:
	if( context->eckey )
		EC_KEY_free( context->eckey );
	if( context->pub_key )
		EC_POINT_free( context->pub_key );
	if( context->signature )
		ECDSA_SIG_free( context->signature );
	free(context);
	return ret;
}

#endif
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


