#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
void tDigest ()
{
	char msg1[] = "Test Message1 ";//待计算摘要的消息1
	char msg2[] = "Test Message2"; //待计算摘要的消息2
	
	unsigned int md_len, i;
	unsigned char md_value[EVP_MAX_MD_SIZE];//保存输出的摘要值的数组
	
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new(); //EVP 消息摘要结构体
	EVP_DigestInit_ex(mdctx,EVP_md5(), NULL); //设置摘要算法和密码算法引擎
	EVP_DigestUpdate(mdctx,msg1, strlen(msg1)); //调用摘要Update计算msg1的摘要
	EVP_DigestUpdate(mdctx,msg2, strlen(msg2)); //调用摘要Update计算msg2的摘要
	EVP_DigestFinal_ex(mdctx, md_value,&md_len);//摘要结束，输出摘要值
	EVP_MD_CTX_free(mdctx);
	printf ("MD5(%s+%s)=\n",msg1, msg2);
	for(i = 0; i < md_len;i++){
		printf ("%02x ", md_value[i]);
	}
	printf ("\n");
	
	char msg3[]="Test Message1 Test Message2";
	EVP_Digest(msg3, strlen(msg3), md_value, &md_len, EVP_md5(), NULL);
	printf ("MD5(%s+%s)=\n",msg1, msg2);
	for(i = 0; i < md_len;i++){
		printf ("%02x ", md_value[i]);
	}
	printf ("\n");
	
}
int main()
{
	OpenSSL_add_all_algorithms();
	tDigest() ;
	return 0;
}
