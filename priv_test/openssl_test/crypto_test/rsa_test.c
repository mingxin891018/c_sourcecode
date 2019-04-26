#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#define OPENSSLKEY "/home/zhaomingxin/private/openssl_test/crypto_test/private.pem"
#define PUBLICKEY "/home/zhaomingxin/private/openssl_test/crypto_test/public.pem"
#define BUFFSIZE 1024
#define log() printf("[%s:%d]\n",__func__, __LINE__)

char* my_encrypt(char *str,char *path_key);//加密
char* my_decrypt(char *str,char *path_key);//解密
void generateKey();//生成RSA私密钥
int main(void){
	char *source="i like dancing !";
	char *ptr_en,*ptr_de;
	
	log();
	//generateKey();
	generateKey_1();

	printf("source is    :%s\n",source);
	ptr_en=my_encrypt(source,PUBLICKEY);
	printf("after encrypt:%s\n",ptr_en);
	ptr_de=my_decrypt(ptr_en,OPENSSLKEY);
	printf("after decrypt:%s\n",ptr_de);
	if(ptr_en!=NULL){
		free(ptr_en);
	}   
	if(ptr_de!=NULL){
		free(ptr_de);
	}   
	return 0;
}

void generateKey() 
{
	/* 生成公钥 */
	//RSA* rsa = RSA_generate_key( 1024, RSA_F4, NULL, NULL);
	RSA* rsa = RSA_generate_key( 1024, 65537, NULL, NULL);
	BIO *bp = BIO_new( BIO_s_file() );
	BIO_write_filename( bp, "public.pem" );
	PEM_write_bio_RSAPublicKey(bp, rsa);
	BIO_free_all( bp );
	
	/* 生成私钥加密存储 */
	//char passwd[]="1234";
	//PEM_write_bio_RSAPrivateKey(bp, rsa, EVP_des_ede3(), (unsigned char*)passwd, 4, NULL, NULL);
	
	bp = BIO_new_file("private.pem", "w+");
	PEM_write_bio_RSAPrivateKey(bp, rsa, NULL, NULL, 0, NULL, NULL);
	BIO_free_all( bp );
	RSA_free(rsa);
}

int generateKey_1(int argc, char *argv[])
{
	/* 产生RSA密钥 */
	RSA *rsa = RSA_generate_key(1024, 65537, NULL, NULL);

	printf("BIGNUM: %s\n", BN_bn2hex(rsa->n));

	/* 提取私钥 */
	printf("PRIKEY:\n");

#if 0
	PEM_write_RSAPrivateKey(stdout, rsa, NULL, NULL, 0, NULL, NULL);
#else
	FILE *fp_pub = fopen(OPENSSLKEY,"w");
	PEM_write_RSAPrivateKey(fp_pub, rsa, NULL, NULL, 0, NULL, NULL);
	fclose(fp_pub);
	fp_pub = NULL;
#endif
	/* 提取公钥 */
	unsigned char *n_b = (unsigned char *)calloc(RSA_size(rsa), sizeof(unsigned char));
	unsigned char *e_b = (unsigned char *)calloc(RSA_size(rsa), sizeof(unsigned char));

	int n_size = BN_bn2bin(rsa->n, n_b);
	int b_size = BN_bn2bin(rsa->e, e_b);

	RSA *pubrsa = RSA_new();
	pubrsa->n = BN_bin2bn(n_b, n_size, NULL);
	pubrsa->e = BN_bin2bn(e_b, b_size, NULL);

	printf("PUBKEY: \n");
#if 0	
	PEM_write_RSAPublicKey(stdout, pubrsa);
#else
	FILE *fp_pri = fopen(PUBLICKEY,"w");
	PEM_write_RSAPublicKey(fp_pri, rsa);
	fclose(fp_pri);
	fp_pri = NULL;

#endif	
	RSA_free(rsa);
	RSA_free(pubrsa);

	return 0;
}

char *my_encrypt(char *str,char *path_key){
	char *p_en;
	RSA *p_rsa;
	FILE *file;
	//int flen = -1;
	int rsa_len = -1;
	if((file=fopen(path_key,"r"))==NULL){
		log();
		return NULL;    
	}
	//p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL);
	p_rsa = PEM_read_RSAPublicKey(file, NULL,NULL,NULL);
	if(p_rsa ==NULL){
		log();
		printf("[%s:%d]PEM_read_RSA_PUBKEY return NULL\n",__func__, __LINE__);
		return NULL;
	}   
	log();
	//flen=strlen(str);
	rsa_len=RSA_size(p_rsa);
	p_en=(unsigned char *)malloc(rsa_len+1);
	memset(p_en,0,rsa_len+1);
	if(RSA_public_encrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_en,p_rsa,RSA_NO_PADDING)<0){
		printf("[%s:%d]RSA_public_encrypt error!!!\n",__func__, __LINE__);
		return NULL;
	}
	log();
	RSA_free(p_rsa);
	fclose(file);
	return p_en;
}
	
char *my_decrypt(char *str,char *path_key){
	char *p_de;
	RSA *p_rsa;
	FILE *file;
	int rsa_len;
	if((file=fopen(path_key,"r"))==NULL){
		log();
		return NULL;
	}
	log();
	if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL){
		log();
		return NULL;
	}
	log();
	rsa_len=RSA_size(p_rsa);
	p_de=(unsigned char *)malloc(rsa_len+1);
	memset(p_de,0,rsa_len+1);
	if(RSA_private_decrypt(rsa_len,(unsigned char *)str,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0){
		return NULL;
	}
	log();
	RSA_free(p_rsa);
	fclose(file);
	return p_de;
}
