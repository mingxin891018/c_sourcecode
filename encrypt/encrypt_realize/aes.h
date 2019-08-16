#ifndef _AES_H_
#define _AES_H_
 
#include <stdint.h>
 
 
// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES128 encryption in CBC-mode of operation and handles 0-padding.
// ECB enables the basic ECB 16-byte block algorithm. Both can be enabled simultaneously.
 
// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif
 
#ifndef ECB
  #define ECB 1
#endif

#define AES_BLOCKSIZE           16

int padding(unsigned char *src, size_t src_len);
void bin_print(const char *data, size_t len);
 
#if defined(ECB) && ECB
 
//加解密 AES_BLOCKSIZE 长度的数据
void AES128_ECB_encrypt(const uint8_t* input, const uint8_t* key, uint8_t *output);
void AES128_ECB_decrypt(const uint8_t* input, const uint8_t* key, uint8_t *output);
 
//加解密任意长度的数据,返回值为加解密之后数据的长度
size_t AES128_ECB_CS5_encrypt(uint8_t *out, uint8_t *in, size_t len, const char *key);
size_t AES128_ECB_CS5_decrypt(uint8_t *out, uint8_t *in, size_t len, const char *key);

#endif // #if defined(ECB) && ECB
 
 
#if defined(CBC) && CBC
 
//加解密 AES_BLOCKSIZE 长度的数据
void AES128_CBC_encrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
void AES128_CBC_decrypt_buffer(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
 
//加解密任意长度的数据,返回值为加解密之后数据的长度
size_t AES128_CBC_CS5_encrypt(uint8_t *out, uint8_t *in, size_t len, const char *key, const char *iv);
size_t AES128_CBC_CS5_decrypt(uint8_t *out, uint8_t *in, size_t len, const char *key, const char *iv);

#endif // #if defined(CBC) && CBC
 
 
 
#endif //_AES_H_
