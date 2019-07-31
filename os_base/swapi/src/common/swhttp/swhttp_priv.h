#ifndef __SWHTTP_PRIV_H__
#define __SWHTTP_PRIV_H__

#include "swhttpclient.h"

#ifdef SUPPORT_HTTPS
#include "swhttpsclient.h"
#endif

#ifdef SUPPORT_OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#ifdef SUPPORT_WOLFSSL
#define X509_V_OK 0
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/error-ssl.h>
#include <wolfssl/ssl.h>
#include <wolfssl/openssl/pem.h>
#include <wolfssl/openssl/ssl.h>  //wolfssl转openssl的兼容层
#include <wolfssl/openssl/crypto.h>
#endif

/*
 *	HTTP客户端
 */
typedef struct http_client
{
	/* 工作套接字 */
	int skt;
	/* 服务器ip，网络字节序 */
	unsigned long ip;
	/* 服务器端口 */
	unsigned short port;
	/* 头域中是否发送Cookie: */
	bool include_Cookie;
	/* 头域中是否发送Accept-Encoding: */
	bool include_Accept_Encoding;
	/* 头域中是否发送Accept-language: */
	bool include_Accept_language;
	/* 头域中是否发送User-Agent: */
	bool include_User_Agent;
	/* 头域中是否发送Pragma: no-cache */
	bool include_Pragma;
	/* 头域中是否发送Cache-Control: no-cache */
	bool include_Cache_Control;
	char sz_cookies[1024];
	/* http版本号1.0,1.1等 */
	char version[8];
	/* http可接收的编码方式 */
	char encoding[32];
	/* http可接收的Accept-language */
	char language[256];
	/* 占用标志 */
	int used;
	/* 服务器ipv6，网络字节序 */
  struct in6_addr ipv6;
	/* http的ETag号*/
	char etag[128];
	char user_agent[128];
	
	char extra[1024];
	/* 连接错误码 */
	int error_code;

	/* 是否是https链接 */
	bool isHttps;
#ifdef SUPPORT_HTTPS
	/*SSL 句柄*/
	SSL_CTX* ctx;
	SSL* ssl;
#endif
	/* 是否shutdonw了 */
	bool shutdown;
}http_client_t;

#define MAX_HTTPCLIENT_NUM		32

void httpsclient_shutdown( HANDLE hHttpclient );

#endif /* __SWHTTP_PRIV_H__ */
