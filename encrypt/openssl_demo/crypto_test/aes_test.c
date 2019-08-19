/*
 *
 * 描述：基于opensll的AES128加密算法的封装
 * 注意：IV向量一定要是数组，且会被写成其他值，每次使用必须重置。
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>

#define SIZE_BLOCK 16

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

//#define DEBUG_PRINT

/*****************************************************************************
* pkcs_type	: 1 PKCS5 	2 PKCS0 
* mode		: 1 CBC 	2 ECB
*****************************************************************************/
unsigned char *AES128_encrypt(const unsigned char *in, size_t len, size_t *out_len, const unsigned char *key, unsigned char *iv, int pkcs_type, int mode);
unsigned char *AES128_decrypt(const unsigned char *in, size_t len, size_t *out_len, const unsigned char *key, unsigned char *iv, int pkcs_type, int mode);

/*****************************************************************************
* Public functions:pkcs5
* PKCS#5
*****************************************************************************/
static unsigned char *pkcs5(const unsigned char *src, size_t src_len, size_t *out_len)
{
	int i = 0;
	unsigned char *p = NULL;
	int padd_len = SIZE_BLOCK - (src_len%SIZE_BLOCK);
	
	p = (unsigned char *)malloc(src_len + padd_len);
	if(!p){
		log_error("malloc error!\n");
		*out_len = 0;
		return NULL;
	}

	memset(p, 0, src_len + padd_len);
	memcpy(p, src, src_len);

	for(i = 0; i < padd_len; ++i)
		p[src_len + i] = padd_len;

	*out_len = src_len + padd_len;
	return p;
}

static unsigned char *pkcs0(const unsigned char *src, size_t src_len, size_t *out_len)
{
	int i = 0;
	unsigned char *p = NULL;
	int padd_len = SIZE_BLOCK - (src_len%SIZE_BLOCK);
	
	if((padd_len - src_len) == SIZE_BLOCK)
		padd_len = 0;

	p = (unsigned char *)malloc(src_len + padd_len);
	if(!p){
		log_error("malloc error!\n");
		*out_len = 0;
		return NULL;
	}

	memset(p, 0, src_len + padd_len);
	memcpy(p, src, src_len);

	for(i = 0; i < padd_len; ++i)
		p[src_len + i] = 0;

	*out_len = src_len + padd_len;
	return p;
}

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;
	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X ", data[i]&0xff);
	printf("\n");
}

unsigned char *AES128_encrypt(const unsigned char *in, size_t len, size_t *out_len, const unsigned char *key, unsigned char *iv, int pkcs_type, int mode)
{
	AES_KEY aes;
	unsigned char *plaintext = NULL, *p = NULL;
	size_t plaintext_len = 0;

	if(!in || !out_len || !key || !iv){
		log_error("param error");
		return NULL;
	}
	if(pkcs_type == 1){
		plaintext = pkcs5(in, len, &plaintext_len);
		if(plaintext == NULL){
			log_error("malloc failed!\n");
			goto end;
		}
	}else if(pkcs_type == 2){
		plaintext = pkcs0(in, len, &plaintext_len);
		if(plaintext == NULL){
			log_error("malloc failed!\n");
			goto end;
		}
	}else{
		log_error("pkcs_type error!\n");
		goto end;
	}

	p = (unsigned char *)malloc(plaintext_len);
	if(p == NULL){
		log_error("malloc failed!\n");
		goto end;
	}

	memset(&aes, 0, sizeof(aes));
	if (AES_set_encrypt_key(key, 128, &aes) < 0) {
		log_error("Unable to set encryption key in AES\n");
		goto end;
	}
#ifdef DEBUG_PRINT
	bin_print(plaintext, plaintext_len);
#endif
	if(mode == 1){
		AES_cbc_encrypt(plaintext, p, plaintext_len, &aes, iv, AES_ENCRYPT);
	}else if(mode == 2){
		size_t i = 0;
		for(i = 0; i < len; i+=SIZE_BLOCK)
			AES_ecb_encrypt(plaintext + i, p + i, &aes, AES_ENCRYPT);
	}else{
		log_error("encryption mode error!\n");
		goto end;
	}
#ifdef DEBUG_PRINT
	bin_print(p, plaintext_len);
#endif
	free(plaintext);
	*out_len = plaintext_len;
	return p;

end:
	if(plaintext)
		free(plaintext);
	if(p)
		free(p);
	*out_len = 0;
	return NULL;
}

unsigned char *AES128_decrypt(const unsigned char *in, size_t len, size_t *out_len, const unsigned char *key, unsigned char *iv, int pkcs_type, int mode)
{
	AES_KEY aes;
	unsigned char *p = NULL;
	
	if(!in || !out_len || !key || !iv){
		log_error("param error");
		return NULL;
	}
	
	if(len%16 || len == 0){
		log_error("ciphertext length error.\n");
		return NULL;
	}

	p = (unsigned char *)malloc(len);
	if(p == NULL){
		log_error("malloc failed!\n");
		return NULL;
	}
	
	memset(p, 0, len);
	memset(&aes, 0, sizeof(aes));
	
	if (AES_set_encrypt_key(key, 128, &aes) < 0) {
		log_error("Unable to set encryption key in AES\n");
		return NULL;
	}

	if (AES_set_decrypt_key(key, 128, &aes) < 0) {
		log_error("Unable to set decryption key in AES\n");
		return NULL;
	}

#ifdef DEBUG_PRINT
	bin_print(in, len);
#endif

	if(mode == 1){
		AES_cbc_encrypt(in, p, len, &aes, iv, AES_DECRYPT);
	}else if(mode == 2){
		size_t i = 0;
		for(i = 0; i < len; i+=SIZE_BLOCK)
			AES_ecb_encrypt(in + i, p + i, &aes, AES_DECRYPT);
	}else{
		log_error("encryption mode error!\n");
		goto end;
	}
	
#ifdef DEBUG_PRINT
	bin_print(p, len);
#endif

	if(p[len - 1] > 16){
		log_error("encryption error!\n");
		goto end;
	}

	if(pkcs_type == 1)
		*out_len = len - p[len - 1];
	else if(pkcs_type == 2){
		*out_len = len;
	}else{
		log_error("encryption mode error!\n");
		goto end;
	}
	return p;

end:
	free(p);
	p = NULL;
	*out_len = 0;
	return NULL;
}

#if 1
int main(int argc, char *argv[])
{
	size_t ciphertext_len = 0, plaintext_len = 0;
	unsigned char *ciphertext = NULL, *plaintext = NULL;
	char *data = "helloworld zhaomingxin";
	char *key = "abcdefghijklmnop";
	unsigned char iv[32] = "0123456789abcdef";

	printf("data= %s\n", data);
	printf("key = %s\n", key);
	printf("iv  = %s\n\n", iv);
	ciphertext = AES128_encrypt((const unsigned char *)data, strlen(data), &ciphertext_len, (const unsigned char *)key, iv, 1, 1);
	if(ciphertext == NULL)
		goto end;
	printf("=========================================================\n");
	printf("ciphertext:\n");
	bin_print(ciphertext, ciphertext_len);
	printf("=========================================================\n");

	printf("\n\n");
	unsigned char iv1[32] = "0123456789abcdef";
	plaintext = AES128_decrypt((const unsigned char *)ciphertext, ciphertext_len, &plaintext_len, (const unsigned char *)key, iv1, 1, 1);
	if(plaintext == NULL)
		goto end;
	log_info("dec success,plaintext_len=%ld\n", plaintext_len);
	printf("=========================================================\n");
	printf("plaintext:\n");
	bin_print(plaintext, plaintext_len);
	printf("=========================================================\n");

end:
	if(ciphertext) 
		free(ciphertext);
	if(plaintext)
		free(plaintext);
	return 0;
}
#endif

