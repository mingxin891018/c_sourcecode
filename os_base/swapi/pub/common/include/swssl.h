/**
 *@file swssl.h
 *@brief 封装的openssl加密和解密算法接口函数
 *@author wurongfu
 *@version 1.0
 *@history
 *        2012-12-02 wurongfu created
 */
#ifndef __SWSSL_H__
#define __SWSSL_H__

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef enum
{
	SWAES_MODE_NULL=0,
	SWAES_MODE_ECB,
	SWAES_MODE_CBC,
}SWAES_MODE;

typedef enum
{
	SWAES_PADDING_NULL=0,
	SWAES_PADDING_PKCS5,
	SWAES_PADDING_PKCS7,
}SWAES_PADDING;

typedef struct
{
	unsigned char key[32];
	int bits;
	int mode;
	unsigned char iv[16];
	int padding;
	int enc;
}swaes_st;

typedef enum//存放key的文件类型
{
	SWRSA_KEYFILE_TYPE_NULL,
	SWRSA_KEYFILE_TYPE_PRIVATE,
	SWRSA_KEYFILE_TYPE_RSAPUBLIC,
	SWRSA_KEYFILE_TYPE_PUBLIC,
}SWRSA_KEYFILE_TYPE;//先遗留

typedef enum
{
	SWRSA_KEY_FROM_BUFFER,
	SWRSA_KEY_FROM_FILE,
}SWRSA_KEY_SOURCR;

typedef struct
{
	SWRSA_KEY_SOURCR type;

	//直接给出：
	unsigned char *n;//公钥，必须给出
	unsigned char *d;//私钥，如果私钥加密则必须给出
	unsigned long e;//公共指数

	//从文件读取:
	char *pubkey_filename;
	SWRSA_KEYFILE_TYPE pubkey_filetype;
	char *prikey_filename;
	SWRSA_KEYFILE_TYPE prikey_filetype;
}swrsa_keyinfo;

typedef enum
{
	SWRSA_PKCS1_PADDING=1, 
	SWRSA_SSLV23_PADDING=2,//不支持
	SWRSA_NO_PADDING=3,
	SWRSA_PKCS1_OAEP_PADDING=4,
	SWRSA_X931_PADDING=5,//不支持
}SWRSA_PADDING;//与openssl保持值一致

typedef enum
{
	SWRSA_ENCRYPT_NULL=0,	
	SWRSA_ENCRYPT_PUBLIC,	
	SWRSA_ENCRYPT_PRIVATE,	
	SWRSA_DECRYPT_PUBLIC,	
	SWRSA_DECRYPT_PRIVATE,	
}SWRSA_ENCRYPT_TYPE;

typedef struct 
{
	void *r;
	unsigned char *n;
	unsigned char *d;
	char *pubkey_filename;
	SWRSA_KEYFILE_TYPE pubkey_filetype;
	char *prikey_filename;	
	SWRSA_KEYFILE_TYPE prikey_filetype;
	unsigned long e;
	int padding;
	int encrypt_type;
}swrsa_st;


#ifdef __cplusplus
extern "C"
{
#endif

/**
 *@brief aes初始化函数
 *@param aesinfo : 自定义的aes结构体
 *@param key: 加密key
 *@param bit: 加密长度，例如128
 *@param mode: 加密模式，例如AES_CBC AES_ECB
 *@param iv: 当加密模式为CBC时为必须，初始化向量
 *@param padding: 填充方式，AES_PKCS5PADDING
 *@param enc:1加密，0解密
 
 *@return  0成功    -1失败 
 */
int sw_aes_init(swaes_st *aesinfo,unsigned char *key, int bits, int mode, unsigned char *iv, SWAES_PADDING padding, int enc);

/**
 *@brief aes加密函数
 *@param aesinfo : 自定义的aes结构体
 *@param out: 如果成功，返回AES加密后的数据，out的空间至少比in的空间大16字节
 *@param in: 待加密的数据
 *@param insize: 待加密的数据长度

 *@return   >0：加密数据的长度，  -1：失败
 */
int sw_aes_enc(swaes_st *aesinfo ,unsigned char *out,unsigned char *in, int insize);

/**
 *@brief aes解密函数
 *@param aesinfo : 自定义的aes结构体
 *@param out: 如果成功，返回AES解密后的数据，out的空间至少比in的空间大16字节
 *@param in: 待解密的数据
 *@param insize: 待解密的数据长度

 *@return >0：解密数据的长度	-1：失败
 */
int sw_aes_dec(swaes_st *aesinfo, unsigned char * out,unsigned char * in, int insize);


/**
 *@brief rsa初始化函数
 *@param rsainfo : 自定义rsa结构体
 *@param key: swrsa_keyinfo结构体
 *@param e: 公共指数
 *@param padding: 自定义填充方式，如SWRSA_PKCS1_PADDING
 *@param type: 自定义加解密方式，例如RSA_ENCRYPT_PUBLIC

 *@return 0--成功    -1 --失败
*/
int sw_rsa_init(swrsa_st *rsainfo, const swrsa_keyinfo *key, int padding,int encrypt_type);


/**
 *@brief rsa加密函数
 *@param rsainfo : rsa信息
 *@param out : 加密后输出buf , >=1024
 *@param in : 输入待加密串 
 *@param insize : 输入待加密串长度

 *@return   >0：加密数据的长度     -1：失败
*/
int sw_rsa_enc(swrsa_st *rsainfo, unsigned char *out, const unsigned char *in, int insize);


/**
 *@brief rsa解密函数
 *@param rsainfo : rsa信息
 *@param out : 解密输出buf, >=2048
 *@param in : 输入加密串 
 *@param insize : 输入加密串长度

 *@return >0:解密后串的长度，   -1：解密失败
*/
int sw_rsa_dec(swrsa_st *rsainfo, unsigned char *out, const unsigned char *in, int insize);


/**
 *@brief sha256加密函数
 *@param out: 如果成功，返回加密后的数据，out的空间至少32字节
 *@param in: 待加密的数据
 *@param insize: 待解密的数据长度

 *@return  >0：加密数据的长度      -1：失败
 */
int sw_sha256_sum(unsigned char *out, unsigned char *in, int insize);

int sw_sha256_file_sum(unsigned char *out,  const char * fname, unsigned int seek);

int sw_sha512_file_sum(unsigned char *out,  const char * fname, unsigned int seek);

int sw_sha512_sum(unsigned char *out, unsigned char *in, int insize);

/**
 *@brief 设置OpenSSL的随机数种子
 */
void sw_ssl_srand(const void *buf,int num);

/**
 *@brief 产生并设置OpenSSL的随机数种子
 */
void sw_ssl_seed(void);

/**
 *@brief 产生随机数0-0x7fffffff
 */
int sw_ssl_random(void);

#ifdef __cplusplus
}
#endif

#endif // __SWSSL_H__
