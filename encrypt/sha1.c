#include "sha1.h"

/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(uint32_t state[5], const uint8_t buffer[64])
{
	uint32_t a, b, c, d, e;
	typedef union {
		unsigned char c[64];
		uint32_t l[16];
	} CHAR64LONG16;

	CHAR64LONG16* block;

#ifdef SHA1HANDSOFF

	static unsigned char workspace[64];
	block = (CHAR64LONG16*)workspace;
	//    NdisMoveMemory(block, buffer, 64);
	memcpy(block, buffer, 64);
#else
	block = (CHAR64LONG16*)buffer;
#endif
	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
}


/* SHA1Init - Initialize new context */

void SHA1Init(SHA1_CTX* context)
{
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Update(SHA1_CTX* context, const unsigned char* data, uint32_t len)
{
	uint32_t i, j;

	j = context->count[0];
	if ((context->count[0] += len << 3) < j)
		context->count[1]++;
	context->count[1] += (len>>29);
	j = (j >> 3) & 63;
	if ((j + len) > 63) {
		//        NdisMoveMemory(&context->buffer[j], data, (i = 64-j));
		memcpy(&context->buffer[j], data, (i = 64-j));
		SHA1Transform(context->state, context->buffer);
		for ( ; i + 63 < len; i += 64) {
			SHA1Transform(context->state, &data[i]);
		}
		j = 0;
	}
	else i = 0;
	//    NdisMoveMemory(&context->buffer[j], &data[i], len - i);
	memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void SHA1Final(SHA1_CTX* context, unsigned char digest[SHA1_HASH_SIZE])
{
	uint32_t i, j;
	unsigned char finalcount[8];

	for (i = 0; i < 8; i++) {
		finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
					>> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
	}
	SHA1Update(context, (unsigned char *)"\200", 1);
	while ((context->count[0] & 504) != 448) {
		SHA1Update(context, (unsigned char *)"\0", 1);
	}
	SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
	for (i = 0; i < 20; i++) {
		digest[i] = (unsigned char)
			((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
	}
	/* Wipe variables */
	i = j = 0;
	//    NdisZeroMemory(context->buffer, 64);
	//    NdisZeroMemory(context->state, 20);
	//    NdisZeroMemory(context->count, 8);
	//    NdisZeroMemory(&finalcount, 8);
	memset(context->buffer, 0x00, 64);
	memset(context->state, 0x00, 20);
	memset(context->count, 0x00, 8);
	memset(&finalcount, 0x00, 8);

#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite its own static vars */
	SHA1Transform(context->state, context->buffer);
#endif
}


#if 1
int8_t aliyun_iot_common_hb2hex(uint8_t hb)    
{
	hb = hb & 0xF;
	return (int8_t) (hb < 10 ? '0' + hb : hb - 10 + 'A');
}

char *bin2string(uint8_t *data, int data_len, char *out)
{                                                                                                                                                                                                                 
	int i = 0;

	if(out){
		for (i = 0; i < data_len; ++i){
			out[i * 2] = aliyun_iot_common_hb2hex(data[i] >> 4); 
			out[i * 2 + 1] = aliyun_iot_common_hb2hex(data[i]);
		}   
	}   
	return out;
}

void main(void)
{
	SHA1_CTX ctx;
	
	char out[SHA1_HASH_SIZE *2 +1] = {0};
	uint8_t digest[SHA1_HASH_SIZE] = {0};

	unsigned char *data = "abcdefghijklmnopqrstuvwxyz";
	int data_len = strlen(data);

	memset(digest, 0, sizeof(digest));
	memset(out, 0, sizeof(out));
	memset(&ctx, 0, sizeof(SHA1_CTX));

	SHA1Init(&ctx);
	SHA1Update(&ctx, data, data_len);
	SHA1Final(&ctx, digest);

	//字符串输出二进制数据
	bin2string(digest, SHA1_HASH_SIZE, out);
	out[SHA1_HASH_SIZE*2] = '\0';
	printf("DATA: %s\n", data);
	printf("---SHA1---\nSHA1=%s\n", out);
}
#endif

