#include <stdio.h>  
#include <openssl/des.h>  
#include <string.h>


//ECB模式
int des_cbe()  
{  
	DES_cblock key;

	//随机密钥  
	//DES_random_key(&key);  
	DES_key_schedule schedule;  
	
	//转换成schedule  
	//DES_set_key_checked(&key, &schedule);   

	memcpy(key, "password", 8);
	DES_set_key_unchecked(&key, &schedule);

	const_DES_cblock input = "hehehe11";  
	DES_cblock output;  

	printf("cleartext: %s\n", input);  

	//加密  
	DES_ecb_encrypt(&input, &output, &schedule, DES_ENCRYPT);  
	printf("Encrypted!\n");  

	printf("ciphertext: ");  
	int i;  
	for (i = 0; i < sizeof(input); i++)  
		printf("%02x", output[i]);  
	printf("\n");  

	//解密  
	DES_ecb_encrypt(&output, &input, &schedule, DES_DECRYPT);  
	printf("Decrypted!\n");  
	printf("cleartext:%s\n", input);  

	return 0;  
} 

//CBC模式
int des_cbc()  
{  
	int i = 0;
	unsigned char *keystring = "this is my key";  
	DES_cblock key;  
	DES_key_schedule key_schedule;  

	memcpy(&key, "12345678", 8);
	DES_set_key_unchecked(&key, &key_schedule);
#if 0
	//生成一个 key  
	//DES_string_to_key(keystring, &key);  
	
	if (DES_set_key_checked(&key, &key_schedule) != 0) {  
		printf("convert to key_schedule failed.\n");  
		return -1;  
	}  
#endif
	//需要加密的字符串  
	unsigned char input[] = "this is a text being encrypted by openssl";  
	size_t len = (sizeof(input)+7)/8 * 8;    
	unsigned char *output = malloc(len+1);  
	memset(output,0,sizeof(len+1));
	//IV  
	DES_cblock ivec;  

	//IV设置为0x0000000000000000  
	memset((char*)&ivec, 1, sizeof(ivec));  

	//加密  
	DES_ncbc_encrypt(input, output, sizeof(input), &key_schedule, &ivec, DES_ENCRYPT);  

	//输出加密以后的内容  
	for (i = 0; i < len; ++i)  
		printf("%02x", output[i]);  
	printf("\n");  

	memset((char*)&ivec, 1, sizeof(ivec));  

	//解密  
	DES_ncbc_encrypt(output, input, len, &key_schedule, &ivec, DES_DECRYPT);  

	printf("%s\n", input);  

	free(output);  
	return 0;
} 



int main()
{
	des_cbe();
	des_cbc();
	return 0;
}
