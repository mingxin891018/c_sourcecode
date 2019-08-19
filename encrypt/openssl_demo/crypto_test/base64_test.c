#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

//注意，openssl里面的base64编码接口会在编码64个字节以后追加一个\n字符进去
int base64_encode(char *in_str, int in_len, char *out_str)
{
	BIO *b64, *bio;
	BUF_MEM *bptr = NULL;
	size_t size = 0;

	if (in_str == NULL || out_str == NULL)
		return -1;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_write(bio, in_str, in_len);
	BIO_flush(bio);

	BIO_get_mem_ptr(bio, &bptr);
	memcpy(out_str, bptr->data, bptr->length);
	size = bptr->length;
	
	BIO_free_all(bio);
	return size;
}

int base64_decode(char *in_str, int in_len, char *out_str)
{
	BIO *b64, *bio;
	int size = 0;

	if (in_str == NULL || out_str == NULL)
		return -1;

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	bio = BIO_new_mem_buf(in_str, in_len);
	bio = BIO_push(b64, bio);

	size = BIO_read(bio, out_str, in_len);

	BIO_free_all(bio);
	return size;
}

int main()
{
	char instr[] = "hello world 1234567890";
	char outstr1[1024] = {0};
	size_t len1 = 0, len2 = 0;
	printf("data=%s\n", instr);
	memset(outstr1, 0, sizeof(outstr1));
	len1 = base64_encode(instr,sizeof(instr),outstr1);
	printf("ciphertext length=%zu,data:%s\n", len1, outstr1);

	char outstr2[1024] = {0};
	memset(outstr2, 0, sizeof(outstr2));
	len2 = base64_decode(outstr1,strlen(outstr1),outstr2);
	printf("plaintext length=%zu,data:=%s\n", len2, outstr2);
	return 0;
}
