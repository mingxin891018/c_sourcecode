/*
 * * dsa.cc
 * * - Show the usage of DSA sign/verify
 * */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/dsa.h>

#define log_info(format,...) printf("[I/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define log_error(format,...) printf("[E/%s:%d] "format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

static void bin_print(const unsigned char *data, size_t len)
{
	size_t i;
	printf("BIN DATA LEN=%zu,DATA:\n", len);
	for(i = 0; i < len; i++)
		printf("%02X", data[i]&0xff);
	printf("\n");
}

int dsa_create_key(void)
{
	int ret = -1;
	DSA* dsa = NULL;
	unsigned char *pub = NULL, *priv = NULL;
	
	// Generate random DSA parameters with 1024 bits
	dsa = DSA_generate_parameters(1024, NULL, 0, NULL, NULL, NULL, NULL);
	if(!dsa){
		log_error("create dsa failed!\n");
		goto DSA_END;
	}

	// Generate DSA keys
	DSA_generate_key(dsa);

	ret = i2d_DSAPublicKey(dsa, &pub);
	log_info("pubkey:\n");
	bin_print(pub, ret);
	ret = i2d_DSAPrivateKey(dsa, &priv);
	log_info("privkey:\n");
	bin_print(priv, ret);


DSA_END:
	if(pub)
		free(pub);
	if(priv)
		free(priv);
	return ret;
}


static unsigned char seed[20] = {
	0xd5, 0x01, 0x4e, 0x4b, 0x60, 0xef, 0x2b, 0xa8, 0xb6, 0x21, 0x1b, 0x40,
	0x62, 0xba, 0x32, 0x24, 0xe0, 0x42, 0x7d, 0xd3,
};

int dsa_test(int argc, char** argv) {
	DSA* dsa;
	unsigned char* input_string;
	unsigned char* sign_string;
	unsigned int sig_len;
	unsigned int i, counter;
	unsigned long h;

	// check usage
	if (argc != 2) {
		fprintf(stderr, "%s <plain text>\n", argv[0]);
		exit(-1);
	}

	// set the input string
	input_string = (unsigned char*)calloc(strlen(argv[1]) + 1,
			sizeof(unsigned char));
	if (input_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for input_string\n");
		exit(-1);
	}
	strncpy((char*)input_string, argv[1], strlen(argv[1]));

	// Generate random DSA parameters with 1024 bits 
#if 1
	dsa = DSA_generate_parameters(1024, NULL, 0, NULL, NULL, NULL, NULL);
#else
	dsa = DSA_new();
	int ret=DSA_generate_parameters_ex(dsa, 512,seed, 20, &counter,&h,NULL);
#endif

	// Generate DSA keys
	DSA_generate_key(dsa);

	// alloc sign_string
	sign_string = (unsigned char*)calloc(DSA_size(dsa), sizeof(unsigned char));    
	if (sign_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for sign_string\n");
		exit(-1);
	}

	// sign input_string
	if (DSA_sign(0, input_string, strlen((char*)input_string), sign_string, &sig_len, dsa) == 0) {
		fprintf(stderr, "Sign Error.\n");
		exit(-1);
	}

	// verify signature and input_string
	int is_valid_signature = DSA_verify(0, input_string, strlen((char*)input_string), sign_string, sig_len, dsa);

	// print
	DSAparams_print_fp(stdout, dsa);
	printf("input_string = %s\n", input_string);
	printf("signed string = ");
	for (i=0; i<sig_len; ++i) {
		printf("%x%x", (sign_string[i] >> 4) & 0xf, 
				sign_string[i] & 0xf);    
	}
	printf("\n");
	printf("is_valid_signature? = %d\n", is_valid_signature);

	return 0;
}



int main(int argc, char *argv[])
{
	dsa_create_key();
	

	dsa_test(argc, argv);
	return 0;
}
