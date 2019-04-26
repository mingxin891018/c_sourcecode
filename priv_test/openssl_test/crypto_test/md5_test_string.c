#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
void test_digest ()
{
	unsigned char md_value[EVP_MAX_MD_SIZE];//保存输出的摘要值的数组
	int md_len, i;
	EVP_MD_CTX mdctx; //EVP 消息摘要结构体
	char msg1[] = "Test Message1";//待计算摘要的消息1
	char msg2[] = "Test Message2"; //待计算摘要的消息2
	EVP_MD_CTX_init(&mdctx);//初始化摘要结构体
	EVP_DigestInit_ex(&mdctx,EVP_md5(), NULL); //设置摘要算法和密码算法引擎
	EVP_DigestUpdate(&mdctx,msg1, strlen(msg1)); //加入需要计算的数据
	EVP_DigestUpdate(&mdctx,msg2, strlen(msg2)); //加入需要计算的数据
	EVP_DigestFinal_ex(&mdctx, md_value,&md_len);//摘要结束，输出摘要值
	EVP_MD_CTX_cleanup(&mdctx); //释放内存
	printf ("MD5\"%s""%s\"=\n",msg1, msg2);
	for(i = 0; i < md_len;i++){
		printf ("%02x ", md_value[i]);
	}
	printf ("\n");
}
int main()
{
	OpenSSL_add_all_algorithms();
	test_digest();
	return 0;
}
