#include "stdafx.h"
#include "swhttpsserver.h"
//#include "swhttpserver.h"
#include "swtcp.h"
#include "swmem.h"
#include "swthrd.h"
#include "swlog.h"
//#include "string_ext.h"
#include "swbase.h"
#include "swcommon_priv.h"

#define SW_CERT "/usr/resource/cert/cert.pem"
#define SW_KEY "/usr/resource/cert/key.pem"

#define sw_malloc malloc
#define sw_free free

typedef struct
{
	/* 工作套接字 */
	int skt;
	/* 服务器ip，网络字节序 */
	unsigned long ip;
	/* 服务器端口 */
	unsigned short port;
	/*服务器线程*/
	HANDLE  thrd;
	/*回调函数*/
	PHttpServerCallback httpserver_callback;
	/*回调函数的参数*/
	unsigned long callback_param;
	/*SSL context*/
	SSL_CTX*  ctx;
}SHttpsServer;

typedef struct
{
	HttpResponseNum type;
	const char *name;
	const char *info;
} HttpEnumString;

static const HttpEnumString httpResponseNames[] = {
	{ HTTP_OK, "OK", "" },
	{ HTTP_MOVED_TEMPORARILY, "Found", "Directories must end with a slash." },
	{ HTTP_REQUEST_TIMEOUT, "Request Timeout",
	  "No request appeared within a reasonable time period." },
	{ HTTP_NOT_IMPLEMENTED, "Not Implemented",
	  "The requested method is not recognized by this server." },
	{ HTTP_UNAUTHORIZED, "Unauthorized", "" },
	{ HTTP_NOT_FOUND, "Not Found",
	  "The requested URL was not found on this server." },
	{ HTTP_BAD_REQUEST, "Bad Request", "Unsupported method." },
	{ HTTP_FORBIDDEN, "Forbidden", "" },
	{ HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error",
	  "Internal Server Error" },
	{ HTTP_CREATED, "Created"," "},
	{ HTTP_ACCEPTED, "Accepted"," "},
	{ HTTP_NO_CONTENT, "No Content"," "},
	{ HTTP_MULTIPLE_CHOICES, "Multiple Choices"," "},
	{ HTTP_MOVED_PERMANENTLY, "Moved Permanently"," "},
	{ HTTP_NOT_MODIFIED, "Not Modified"," " },
	{ HTTP_BAD_GATEWAY, "Bad Gateway", " " },
	{ HTTP_SERVICE_UNAVAILABLE, "Service Unavailable", " " },
};
static const char RFC1123FMT[] = "%a, %d %b %Y %H:%M:%S GMT";

static bool HttpsServerProc( unsigned long wParam, unsigned long lParam );

/** 
 * 开启一个httpserver 
 * 
 * @param port 服务器端口号， 网络序
 * @param callback 回调函数
 * @param wparam  回调函数的参数
 * 
 * @return 返回Httpserver句柄
 */
HANDLE sw_httpsserver_open(unsigned short port, PHttpServerCallback callback, uint32_t wparam )
{
	int fd;
	int on = 1;
	SHttpsServer* pServer = NULL;
	
	if((pServer = sw_malloc(sizeof(SHttpsServer))) == NULL)
	{
		BASE_LOG_FATAL("[HTTPSERVER]sw_malloc failed!\n");
		return NULL;			
	}

	/* SSL Init*/
	SSL_library_init();

  /* Load SSL all error message */
  SSL_load_error_strings();

	/*Create a SSL_CTX by SSLv23_server_method(), (SSL Content Text)*/
  pServer->ctx = SSL_CTX_new(SSLv23_server_method());
  if (pServer->ctx == NULL) 
	{
		BASE_LOG_FATAL("SSL_CTX_new SSLV23_server_method failed!!!\n");
		sw_free(pServer);
		return NULL;
  }
	BASE_LOG_INFO("+++++++++SSL_CTX_new  SSLV23_server_method success+++++++++++\n");

	/*Load certificate_file*/
  if (SSL_CTX_use_certificate_file( pServer->ctx, SW_CERT, SSL_FILETYPE_PEM) <= 0) 
	{
		BASE_LOG_FATAL("SSL_CTX_use_certificate_file failed!!!\n");
		sw_free(pServer);
		return NULL;
  }
	BASE_LOG_INFO("+++++++++sw_ssl_ctx_use_certificate_file ok++++++++++++++++++\n");

	/*Load PrivateKey_file*/
  if (SSL_CTX_use_PrivateKey_file(pServer->ctx, SW_KEY, SSL_FILETYPE_PEM) <= 0) 
	{
		BASE_LOG_FATAL("SSL_CTX_use_PrivateKey_file failed!!!\n");
		sw_free(pServer);
		return NULL;
  }
	BASE_LOG_INFO("+++++++++sw_ssl_ctx_use_PrivateKey_file ok++++++++++++++++++\n");

	/*Check private key*/
  if (!SSL_CTX_check_private_key(pServer->ctx)) 
	{
		BASE_LOG_FATAL("SSL_CTX_check_private_key failed!!!\n");
		sw_free(pServer);
		return NULL;
  }
	BASE_LOG_INFO("+++++++++sw_ssl_ctx_check_private_key ok++++++++++++++++++\n");

	fd = sw_tcp_socket();
	if(fd < 0)
	{
		BASE_LOG_FATAL("sw_httpserver_open::Create socket failed!\n");
		sw_free(pServer);
		return NULL;
	}

#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (void *)&on, sizeof(on)) ;
#else
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)) ;
#endif

	sw_tcp_bind(fd, INADDR_ANY, port);
	sw_tcp_listen(fd, 5);

	memset(pServer, 0, sizeof(SHttpsServer));
	pServer->skt = fd;
	pServer->ip = INADDR_ANY;
	pServer->port = port;
	
	pServer->httpserver_callback = callback;
	pServer->callback_param = wparam;
	pServer->thrd = sw_thrd_open( "tHttpsServer", 80, 0, 16384, (PThreadHandler)HttpsServerProc, (unsigned long)pServer, 0 );

	if(pServer->thrd == NULL)
	{
		BASE_LOG_FATAL("[HTTPSERVER]sw_thrd_open failed!\n");
		sw_free(pServer);
		return NULL;	
	}

	sw_thrd_resume( pServer->thrd);

	return pServer;
}


/** 
 * 关闭httpserver
 * 
 * @param server 
 */
void sw_httpsserver_close( HANDLE server )
{
	SHttpsServer* pServer = (SHttpsServer*)server;
	if(pServer->skt != -1)
	{
		shutdown(pServer->skt,2);
	}
	if(pServer->thrd)
	{
		sw_thrd_close(pServer->thrd, 5000);
		pServer->thrd = NULL;
	}
	pServer->httpserver_callback = NULL;
	pServer->callback_param = 0;
	
	sw_tcp_close(pServer->skt);
	pServer->skt = -1;
  SSL_CTX_free( pServer->ctx );
	sw_free(pServer);	
}


static int getLine(SHttpConnectObj* obj)
{
	int  count = 0;
	char *buf = obj->buf;
	int len = 0;

	len = SSL_read((SSL*)(obj->ssl), buf + count, 1);
	while (len == 1)
	{
		obj->request_header.header_length++;
	  
		if (buf[count] == '\r') 
			continue;
		if (buf[count] == '\n') 
		{
			buf[count] = 0;
			return count;
		}
		if(count < (HTTPSERVER_MAX_LINE - 1))      /* check owerflow */
			count++;
	}
	if (count) 
		return count;
	else 
		return -1;
}

/** 
 * 接收Request报头， 成功则填充request_header结构
 * 
 * @param obj 
 * @param timeout 
 * 
 * @return int: 成功返回报文头的大小， 失败返回负数
 */
int sw_httpsserver_recv_request_header(SHttpConnectObj* obj, int timeout )
{
	char* p = NULL, *last = NULL;
	
	printf("obj->buf:%s\n",obj->buf);
	memset(&obj->request_header, 0, sizeof(HTTP_REQUEST_HEADER));
	
	do
	{
		if(getLine(obj) <= 0)
			break;
		
		if(strncasecmp(obj->buf, "GET", strlen("GET")) == 0 || strncasecmp(obj->buf, "POST", strlen("POST")) == 0 )
		{
			p = strtok_r(obj->buf, " ", &last);
			if(p == NULL)
				continue;
			strlcpy(obj->request_header.method, p, sizeof(obj->request_header.method));
			p = strtok_r(obj->buf, " ", &last);
			if(p == NULL)
				continue;

			strlcpy(obj->request_header.request_url, p, sizeof(obj->request_header.request_url));		
		}
		else if(strncasecmp(obj->buf, "Host:", strlen("Host:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.host, p+1, sizeof(obj->request_header.host));
		}
		else if(strncasecmp(obj->buf, "Accept:", strlen("Accept:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.accept_type, p+1, sizeof(obj->request_header.accept_type));		
		}
		else if(strncasecmp(obj->buf, "Accept-Encoding:", strlen("Accept-Encoding:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.accept_encoding, p+1, sizeof(obj->request_header.accept_encoding));			
		}	
		else if(strncasecmp(obj->buf, "Content-Type:", strlen("Content-Type:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.content_type, p+1, sizeof(obj->request_header.content_type));			
		}
		else if(strncasecmp(obj->buf, "Content-Length:", strlen("Content-Length:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			obj->request_header.content_length = atoi(p+1);			
		}	
		else if(strncasecmp(obj->buf, "Connection:", strlen("Connection:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.connection, p+1,  sizeof(obj->request_header.connection));			
		}			
#ifdef HTTP_AUTHENTICATION_REQUIRED
		else if(strncasecmp(obj->buf, "Authorization:", strlen("Authorization:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strlcpy(obj->request_header.authorization, p+1,  sizeof(obj->request_header.authorization));
		}
#endif
	}while(1);

	BASE_LOG_DEBUG("[HTTPSERVER GET REQUEST]\n");
	BASE_LOG_DEBUG("[METHOD]:%s\n", obj->request_header.method);
	BASE_LOG_DEBUG("[URL]:%s\n", obj->request_header.request_url);
	BASE_LOG_DEBUG("[HOST]:%s\n", obj->request_header.host);
	BASE_LOG_DEBUG("[ACCEPT_TYPE]:%s\n", obj->request_header.accept_type);
	BASE_LOG_DEBUG("[CONNECTION]:%s\n", obj->request_header.connection);
	BASE_LOG_DEBUG("[CONTENT_TYPE]:%s\n", obj->request_header.content_type);
	BASE_LOG_DEBUG("[CONTENT_LENGHT]:%d\n", obj->request_header.content_length);

	return obj->request_header.header_length;	
}
	
/** 
 * 接收请求实体
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return 返回接收数据的字节数， 0: 接收完成， -1: 接收出错
 */
int sw_httpsserver_recv_request_content(SHttpConnectObj* obj, char*buf, int buf_size, int timeout )
{
	if(obj->request_header.content_length > 0)
	{
		return SSL_read((SSL*)(obj->ssl), buf, buf_size);
	}
	return 0;
}

/** 
 * 
 * 发送响应报文头
 * @param obj 
 * @param responseNum 
 * @param pContentType 
 * @param pContentEncoding 
 * @param pConnection 
 * @param nContentLength 
 * @param timeout 
 * 
 * @return 发送的数据字节数
 */
int sw_httpsserver_send_response_header(SHttpConnectObj* obj, HttpResponseNum responseNum, 
									   char* pContentType, char* pContentEncoding, 
									   char* pConnection,
									   int nContentLength, 
									   int timeout)
{
	if (obj == NULL)
		return -1;
	char *buf = obj->buf;
	int size = sizeof(obj->buf);
	const char *responseString = "";
	const char *infoString = NULL;
	time_t timer = time(0);
	char timeStr[80];
	int i, len;

	for (i = 0; i < (int)(sizeof(httpResponseNames)/sizeof(httpResponseNames[0])); i++) 
	{
		if (httpResponseNames[i].type == responseNum) 
		{
			responseString = httpResponseNames[i].name;
			infoString = httpResponseNames[i].info;
			break;
		}
	}

	if(infoString == NULL)
	{
		BASE_LOG_FATAL("Wrong response number[%d]\n",  responseNum);
		return -1;
	}

	/* emit the current date */
	strftime(timeStr, sizeof(timeStr), RFC1123FMT, gmtime(&timer));

	len = snprintf(buf, size, 
				  "HTTP/1.1 %d %s\r\n"
				  "Date: %s\r\n",
				  responseNum, responseString, timeStr);
#ifdef HTTP_AUTHENTICATION_REQUIRED
	if(responseNum == HTTP_UNAUTHORIZED)
	{
		srand( (unsigned)time( NULL ) + rand()*2 );
		len += snprintf(buf+len, len<size ? size-len : 0, "WWW-Authenticate: Digest realm=\"HttpDigestAuthentication\", qop=\"auth\", nonce=\"%d\", opaque=\"abcd01082883008ab01082883008abcd\"\r\n", rand());
	}
#endif
	if(pContentType)
		len += snprintf(buf+len, len<size ? size-len : 0, "Content-type: %s\r\n", pContentType);
	if(pContentEncoding)
		len += snprintf(buf+len, len<size ? size-len : 0, "Content-Encoding: %s\r\n", pContentEncoding);
	if(nContentLength)
		len += snprintf(buf+len, len<size ? size-len : 0, "Content-Length: %d\r\n", nContentLength);
	if(pConnection)
		len += snprintf(buf+len, len<size ? size-len : 0, "Connection: %s\r\n", pConnection);
    
	len += snprintf(buf+len, len<size ? size-len : 0, "\r\n");
	
	BASE_LOG_DEBUG("[HTTPSERVER RESPONSE]%d\n%s\n",len, buf);
	if (len > size)
		return -1;
	i = 0;
	while(i < len)
		i += SSL_write((SSL*)(obj->ssl), buf + i, len - i);

	return len;
}
	
/** 
 * 发送响应实体
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return 返回发送数据字节数 -1：发送失败
 */
int sw_httpsserver_send_response_content(SHttpConnectObj* obj,
										char*buf, int buf_size,
										int timeout)
{
	return SSL_write((SSL*)(obj->ssl), buf, buf_size);
}

/** 
 * 关闭连接,同时释放资源
 * 
 * @param obj 
 */
void sw_httpsserver_close_connectobj(SHttpConnectObj* obj)
{
	if(obj)
	{	
		SSL_shutdown((SSL*)(obj->ssl));
		SSL_free((SSL*)(obj->ssl));

		sw_tcp_close(obj->skt);
		sw_free(obj);	
	}
}

static bool HttpsServerProc( unsigned long wParam, unsigned long lParam )
{
	unsigned long accept_ip;
	unsigned short port;
	int skt = -1;
	X509* client_cert = NULL;
	SSL* ssl = NULL;
	int on = 1;
	char* str = NULL;
	SHttpsServer *pServer = (SHttpsServer *)wParam;
	{
		{
			if((skt = sw_tcp_accept(pServer->skt, &accept_ip, &port)) >= 0)
			{
				SHttpConnectObj* obj = sw_malloc(sizeof(SHttpConnectObj));
				
				if(obj == NULL)
				{
					BASE_LOG_FATAL("[HTTPSERVER]sw_malloc failed!\n");
					pServer->thrd = NULL;
					return false;
				}
				setsockopt(skt, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof (on));
				
				obj->skt = skt;
				obj->from_ip = accept_ip;
				obj->from_port = port;
				obj->hHttpServer = pServer;

				ssl = SSL_new(pServer->ctx);
				SSL_set_fd(ssl, skt);
				obj->ssl = (HANDLE)ssl;

				if (SSL_accept(ssl) == -1) 
				{
					BASE_LOG_FATAL("SSL_accept failed !!!\n");
					sw_free(obj);
					return  true;
				}
	
				/* Get the cipher - opt */
				str = (char*)SSL_get_cipher( ssl);
				if(str)
					BASE_LOG_INFO("SSL connection using %s\n",str);
				else
					BASE_LOG_FATAL("Can't get cipher!!! \n");
  
  			/*Get client's certificate (note: beware of dynamic allocation) - opt */
				client_cert = SSL_get_peer_certificate (ssl);
 				if (client_cert != NULL ) 
				{
    			BASE_LOG_INFO("Client certificate:\n");
   			 	str = X509_NAME_oneline (X509_get_subject_name (client_cert), 0, 0);
    			if(str)
					{	
    				BASE_LOG_INFO("\t subject: %s\n", str);
    				OPENSSL_free (str);
    			}

    			str = X509_NAME_oneline (X509_get_issuer_name  (client_cert), 0, 0);
    			if(str)
					{
    				BASE_LOG_INFO ("\t issuer: %s\n", str);
    				OPENSSL_free (str);
    			}
   				 /* We could do all sorts of certificate verification stuff here before
       		deallocating the certificate. */
    
    			X509_free (client_cert);
  			} 
				else
				{
    			BASE_LOG_INFO("Client does not have certificate.\n");
				}
				
				if(pServer->httpserver_callback)
					pServer->httpserver_callback(obj, pServer->callback_param);
				
				return true;
			}
		}
	}
	
	pServer->thrd = NULL;
	return false;
}

