#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#define RSA_PRIKEY "./rsa_private.pem"
#define RSA_PUBKEY "./rsa_public.pem"
#define BUFFSIZE 1024

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

int generateKey(void);//生成RSA私密钥
unsigned char* RSA_encrypt(const unsigned char *str,char *path_key);//加密
unsigned char* RSA_decrypt(const unsigned char *str,char *path_key);//解密

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;
	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X ", data[i]&0xff);
	printf("\n");
}

int generateKey(void) 
{
	/* 生成公钥 */
	//RSA* rsa = RSA_generate_key( 1024, RSA_F4, NULL, NULL);
	
	/* openssl 1.1.1 */
	int ret = 0;
	RSA *rsa = RSA_new();
	BIGNUM* bne = BN_new();

	ret=BN_set_word(bne,RSA_F4);
	ret = RSA_generate_key_ex(rsa,1024,bne,NULL);
	if(ret != 1){
		log_error("create rsa failed!\n");
		goto err;
	}

	BIO *bp = BIO_new( BIO_s_file() );
	BIO_write_filename( bp, RSA_PUBKEY);
	PEM_write_bio_RSAPublicKey(bp, rsa);
	BIO_free_all( bp );
	
	/* 生成私钥加密存储 */
	//char passwd[]="1234";
	//PEM_write_bio_RSAPrivateKey(bp, rsa, EVP_des_ede3(), (unsigned char*)passwd, 4, NULL, NULL);
	
	bp = BIO_new_file(RSA_PRIKEY, "w+");
	PEM_write_bio_RSAPrivateKey(bp, rsa, NULL, NULL, 0, NULL, NULL);

err:
	BIO_free_all( bp );
	RSA_free(rsa);
	return ret;
}

unsigned char *RSA_encrypt(const unsigned char *str,char *path_key){
	unsigned char *p_en = NULL;
	RSA *p_rsa = NULL;
	FILE *file = NULL;
	int rsa_len = -1;
	if((file=fopen(path_key,"r"))==NULL){
		return NULL;    
	}
	//p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL);
	p_rsa = PEM_read_RSAPublicKey(file, NULL,NULL,NULL);
	if(p_rsa ==NULL){
		log_error("read rsa public key error!\n");
		return NULL;
	}   
	rsa_len=RSA_size(p_rsa);
	p_en=(unsigned char *)malloc(rsa_len+1);
	memset(p_en,0,rsa_len+1);
	log_info("rsa_len=%d\n", rsa_len);
	if(RSA_public_encrypt(rsa_len, str, (unsigned char*)p_en, p_rsa, RSA_NO_PADDING) < 0){
		log_error("RSA_public_encrypt error!!!\n");
		return NULL;
	}

	RSA_free(p_rsa);
	fclose(file);
	return p_en;
}
	
unsigned char *RSA_decrypt(const unsigned char *str,char *path_key){
	unsigned char *p_de = NULL;
	RSA *p_rsa = NULL;
	FILE *file = NULL;
	int rsa_len = 0;
	if((file=fopen(path_key,"r"))==NULL){
		return NULL;
	}
	if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL){
		return NULL;
	}
	rsa_len = RSA_size(p_rsa);
	p_de = (unsigned char *)malloc(rsa_len+1);
	memset(p_de, 0, rsa_len+1);
	if(RSA_private_decrypt(rsa_len, str, (unsigned char*)p_de, p_rsa, RSA_NO_PADDING) < 0){
		return NULL;
	}

	RSA_free(p_rsa);
	fclose(file);
	return p_de;
}

#if 1
int main(int argc, char *argv[])
{
	const char *source = "hello world zhaomingxin";
	unsigned char *ptr_en = NULL, *ptr_de = NULL;
	
	generateKey();

	log_info("source is    :%s\n",source);
	
	ptr_en=RSA_encrypt((const unsigned char *)source, RSA_PUBKEY);
	log_info("after encrypt:\n");
	bin_print((const unsigned char *)ptr_en, 1024/8);

	ptr_de=RSA_decrypt(ptr_en, RSA_PRIKEY);
	log_info("after decrypt:%s\n", ptr_de);
	bin_print((const unsigned char *)ptr_de, 1024/8);
	
	if(ptr_en!=NULL){
		free(ptr_en);
	}   
	if(ptr_de!=NULL){
		free(ptr_de);
	}   
	return 0;
}
#endif

