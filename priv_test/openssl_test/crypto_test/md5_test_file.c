#include<stdio.h>
#include<string.h>
#include<openssl/evp.h>
int DigestFile(char *infile, unsigned char*alg)
{
	unsigned char md_value[EVP_MAX_MD_SIZE];
	int i;
	unsigned int md_len;
	EVP_MD_CTX mdctx;
	FILE *fpIn;
	int inl;
	unsigned char in[1024];
	const EVP_MD *evpMd;
	if(strcmp((const char*)alg,"md5") == 0)
	{
		evpMd = EVP_md5();
	}
	else if(strcmp((const char*)alg,"sha1") == 0)
	{
		evpMd = EVP_sha1();
	}
	else
	{
		printf ("Err Code [13]: Incorrect type of argument, md5 or sha1 accepted .\n");
		return 13;
	}
	fpIn = fopen( infile , "rb");
	if (fpIn == NULL)
	{
		printf ("Err Code [12]: Open file %s for read err , check the file path .\n", infile );
		return 12;
	}
	EVP_MD_CTX_init(&mdctx);
	EVP_DigestInit_ex(&mdctx, evpMd, NULL);
	while(1)
	{
		inl=fread(in,1,1024,fpIn);
		if ( inl <= 0)
			break;
		EVP_DigestUpdate(&mdctx, in, inl);
	}
	EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	EVP_MD_CTX_cleanup(&mdctx);
	printf ("%s<%s> = ", alg, infile );
	for(i = 0; i < md_len; i++)
	{
		printf ("%02x", md_value[i]);
	}
	printf ("\n");
	fclose (fpIn);
	return 0;
}
int main(int argc, char*argv[])
{

	if (argc != 3)
	{
		printf ("Err code [11]: Incorrect number of args, 2 args received .\n");
		return 11;
	}
	OpenSSL_add_all_algorithms();
	DigestFile(argv[2],(unsigned char *)argv[1]);
	return 0;
}
