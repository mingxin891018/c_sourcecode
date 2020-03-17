#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/bn.h>
#include <openssl/dsa.h>
#include <openssl/err.h>

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

DSA *dsa_create_key(unsigned char **pubkey, int *pubkey_len, unsigned char **privkey, int *privkey_len, 
		unsigned char **p, int * p_len, unsigned char **q, int *q_len, unsigned char **g, int *g_len);

int dsa_get_sign_use_privkey(unsigned char *privkey, int privkey_len, unsigned char **sign_buf, int *sign_len);

int dsa_verify_sign(unsigned char *dgst, int dgst_len, unsigned char *sign_buf, int sign_len);

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;
	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X ", data[i]&0xff);
	printf("\n");
}

DSA *dsa_create_key(unsigned char **pubkey, int *pubkey_len, unsigned char **privkey, int *privkey_len, 
		unsigned char **p, int * p_len, unsigned char **q, int *q_len, unsigned char **g, int *g_len)
{
	int ret = 0;

	DSA *dsa = DSA_generate_parameters(1024, NULL, 0, NULL, NULL, NULL, NULL);
	if(!dsa)
	{
		log_error("create dsa with parameters failed!\n");
		goto END;
	}

	if(DSA_generate_key(dsa) != 1)
	{
		log_error("generate dsa failed!\n");
		goto END;
	}
	if(pubkey != NULL)
	{
		BIGNUM *bg_pubkey = DSA_get0_pub_key(dsa);
		if(!bg_pubkey)
		{
			log_error("get bignum pubkey failed!\n");
			goto END;
		}

		*pubkey_len = BN_num_bytes(bg_pubkey);
		if(*pubkey_len <= 0)
		{
			log_error("get pubkey length failed!\n");
			goto END;
		}

		*pubkey = malloc(*pubkey_len);
		if(!(*pubkey))
		{
			log_error("malloc pubkey failed!\n");
			goto END;
		}
		memset(*pubkey, 0, *pubkey_len);
		ret = BN_bn2bin(bg_pubkey, *pubkey);
		log_info("pubkey_len=%d,BN_bn2bin ret=%d\n", *pubkey_len, ret);
	}

	if(privkey != NULL)
	{
		BIGNUM *bg_privkey = DSA_get0_priv_key(dsa);
		if(!bg_privkey)
		{
			log_error("get bignum privkey failed!\n");
			goto END;
		}
		
		*privkey_len = BN_num_bytes(bg_privkey);
		if(*privkey_len <= 0)
		{
			log_error("get privkey length failed!\n");
			goto END;
		}

		*privkey = malloc(*privkey_len);
		if(!(*privkey))
		{
			log_error("malloc privkey failed!\n");
			goto END;
		}
		memset(*privkey, 0, *privkey_len);
		ret = BN_bn2bin(bg_privkey, *privkey);
		log_info("privkey_len=%d,BN_bn2bin ret=%d\n", *privkey_len, ret);

	}

	if(p&&g&&q)
	{
		BIGNUM *bg_p, *bg_q, *bg_g; 
			DSA_get0_pqg(dsa, &bg_p, &bg_q, &bg_g); 

		*p_len = BN_num_bytes(bg_p);
		if(*p_len <= 0)
		{
			log_error("get p length failed!\n");
			goto END;
		}

		*p = malloc(*p_len);
		if(!(*p))
		{
			log_error("malloc p failed!\n");
			goto END;
		}
		memset(*p, 0, *p_len);
		ret = BN_bn2bin(bg_p, *p);
		log_info("p_len=%d,BN_bn2bin ret=%d\n", *p_len, ret);

		*g_len = BN_num_bytes(bg_g);
		if(*g_len <= 0)
		{
			log_error("get g length failed!\n");
			goto END;
		}

		*g = malloc(*g_len);
		if(!(*g))
		{
			log_error("malloc g failed!\n");
			goto END;
		}
		memset(*g, 0, *g_len);
		ret = BN_bn2bin(bg_g, *g);
		log_info("g_len=%d,BN_bn2bin ret=%d\n", *g_len, ret);

		*q_len = BN_num_bytes(bg_q);
		if(*q_len <= 0)
		{
			log_error("get q length failed!\n");
			goto END;
		}

		*q = malloc(*q_len);
		if(!(*q))
		{
			log_error("malloc q failed!\n");
			goto END;
		}
		memset(*q, 0, *q_len);
		ret = BN_bn2bin(bg_q, *q);
		log_info("q_len=%d,BN_bn2bin ret=%d\n", *q_len, ret);
	}
	return dsa;

END:
	if(dsa)
		DSA_free(dsa);
	return NULL;
}



int main(void)
{
	unsigned char sign_buf[256] = {0};
	unsigned char *pubkey = NULL, *privkey = NULL, *p , *q, *g;
	int pubkey_len = 0, privkey_len = 0, p_len, q_len, g_len, sign_len = 0;
	DSA *dsa = dsa_create_key(&pubkey, &pubkey_len, &privkey, &privkey_len, &p, &p_len, &q, &q_len, &g, &g_len);
	log_info("pubkey_len=%d,privkey_len=%d\n", pubkey_len, privkey_len);
	bin_print(pubkey, pubkey_len);
	bin_print(privkey, privkey_len);

	DSA *dsa1 = DSA_new();
	DSA *dsa2 = DSA_new();
	log_info("==debug===\n");
	
	//dsa1
	BIGNUM *bg_p = BN_new();
	BIGNUM *bg_g = BN_new();
	BIGNUM *bg_q = BN_new();
	BN_bin2bn(p, p_len, bg_p);
	BN_bin2bn(g, g_len, bg_g);
	BN_bin2bn(q, q_len, bg_q);
	DSA_set0_pqg(dsa1, bg_p, bg_q, bg_g);
	DSA_generate_key(dsa1);
	log_info("==debug===\n");

	//dsa2
	BIGNUM *bg_pubkey = BN_new();
	BIGNUM *bg_privkey = BN_new();;
	BN_bin2bn(pubkey, pubkey_len, bg_pubkey);
	BN_bin2bn(privkey, privkey_len, bg_privkey);
	DSA_set0_key(dsa2, NULL, bg_privkey);
	log_info("==bg_pubkeydebug===\n");
	DSA_generate_key(dsa2);
	log_info("==bg_pubkeydebug===\n");

	//dsa1
	DSA_sign(0, "1234567890", 10, sign_buf, &sign_len, dsa1);
	log_info("==debug===\n");

	//dsa2
	int ret = DSA_verify(0, "1234567890", 10, sign_buf,sign_len, dsa2);
	if(ret == 1)
	{
		log_info("============verify ok ==========\n");
	}

	if(p)
		free(p);
	if(g)
		free(g);
	if(q)
		free(q);

	if(pubkey)
		free(pubkey);
	if(privkey)
		free(privkey);

	if(dsa1)
		DSA_free(dsa1);
	if(dsa2)
		DSA_free(dsa2);

	if(bg_pubkey)
		BN_free(bg_pubkey);
	if(bg_privkey)
		BN_free(bg_privkey);
	if(bg_p)
		BN_free(bg_p);
	if(bg_g)
		BN_free(bg_g);
	if(bg_q)
		BN_free(bg_q);
	return 0 ;
}


