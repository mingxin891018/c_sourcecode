/**
 * @file   swhttpserver.h
 * @version %I% %G%
 * @author Hongchen Dou
 * @brief  http server
 * @history
 * 				2007-07-15  Houchen Dou created
 * 				2010-07-29 	Shihui Qu modified
 */

#ifndef __SWHTTPSERVER_H__
#define __SWHTTPSERVER_H__

typedef enum
{
  HTTP_OK = 200,
  HTTP_MOVED_TEMPORARILY = 302,
  HTTP_BAD_REQUEST = 400,       /* malformed syntax */
  HTTP_UNAUTHORIZED = 401, /* authentication needed, respond with auth hdr */
  HTTP_NOT_FOUND = 404,
  HTTP_FORBIDDEN = 403,
  HTTP_REQUEST_TIMEOUT = 408,
  HTTP_NOT_IMPLEMENTED = 501,   /* used for unrecognized requests */
  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_CONTINUE = 100,
  HTTP_SWITCHING_PROTOCOLS = 101,
  HTTP_CREATED = 201,
  HTTP_ACCEPTED = 202,
  HTTP_NON_AUTHORITATIVE_INFO = 203,
  HTTP_NO_CONTENT = 204,
  HTTP_MULTIPLE_CHOICES = 300,
  HTTP_MOVED_PERMANENTLY = 301,
  HTTP_NOT_MODIFIED = 304,
  HTTP_PAYMENT_REQUIRED = 402,
  HTTP_BAD_GATEWAY = 502,
  HTTP_SERVICE_UNAVAILABLE = 503, /* overload, maintenance */
  HTTP_RESPONSE_SETSIZE=0x7fffffff
} http_response_num_t;

#define HTTPSERVER_MAX_LINE 1024

#ifndef HTTP_AUTHENTICATION_REQUIRED
#define HTTP_AUTHENTICATION_REQUIRED
#endif

typedef struct
{
	char method[8];
	char request_url[1024];
	char host[128];
	char accept_type[16];
	char accept_encoding[16];
	char connection[16];
#ifdef HTTP_AUTHENTICATION_REQUIRED
	char authorization[256];
#endif	
	char content_type[16];
	int content_length;
	int header_length;
}http_request_header_t;

	
typedef struct
{
	/* 工作套接字 */
	int skt;
	/* 客户端ip，网络字节序 */
	unsigned long from_ip;
	/* 客户端ipv6，网络字节序 */
	//struct in6_addr from_ipv6;
	/* 客户端端口 */
	unsigned short from_port;
	/* 服务器句柄 */
	HANDLE h_http_server;
	/* 工作缓冲区 */
	http_request_header_t request_header;

	char buf[HTTPSERVER_MAX_LINE];
	/*SSL 句柄*/
	HANDLE ssl;
}http_connect_obj_t;


#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 定义HttpServer回调函数,触发时机:接收一个新的连接时, 接收新连接，
 *        同时产生一个连接实例(内部分配内存，需要手工调用sw_httpserver_close_connectobj释放）
 *
 * @param obj 连接体 
 * @param wparam  附加参数
 *
 * @return 
 */
typedef int (*http_server_callback)( http_connect_obj_t* obj, uint32_t wparam ); 

/** 
 * @brief 开启一个httpserver 
 * 
 * @param port 服务器端口号， 网络序
 * @param callback 回调函数
 * @param wparam  回调函数的参数
 * 
 * @return 成功，返回Httpserver句柄；失败，返回NULL
 */
HANDLE sw_httpserver_open(unsigned short port, http_server_callback callback, uint32_t wparam );
HANDLE sw_httpserver_open_v6(unsigned short port, http_server_callback callback, uint32_t wparam );


/** 
 * @brief 关闭httpserver
 * 
 * @param server httpserver句柄
 */
void sw_httpserver_close( HANDLE server );

/** 
 * @brief 接收Request报头，成功则填充request_header结构
 * 
 * @param obj 连接体
 * @param timeout 超时
 * 
 * @return 成功返回报文头的大小， 失败返回负数
 */
int sw_httpserver_recv_request_header(http_connect_obj_t* obj, int timeout );
	
/** 
 * @brief 接收请求实体
 * 
 * @param obj 连接体
 * @param buf 接收缓冲区
 * @param buf_size 缓冲区大小
 * @param timeout 超时
 * 
 * @return 返回接收数据的字节数，0: 接收完成，-1: 接收出错
 */
int sw_httpserver_recv_request_content(http_connect_obj_t* obj, char*buf, int buf_size, int timeout );

/** 
 * @brief 发送响应报文头
 *
 * @param obj 连接体
 * @param response_num 响应值  
 * @param content_type 内容类型
 * @param content_encoding 
 * @param connection 
 * @param content_lenth 内容长度
 * @param timeout 超时
 * 
 * @return 成功，返回发送的数据字节数；失败，返回-1
 */
int sw_httpserver_send_response_header(http_connect_obj_t* obj, http_response_num_t response_num, 
									   char* content_type, char* content_encoding, 
									   char* connection,
									   int content_length, 
									   int timeout);
	
/** 
 * @brief 发送响应实体
 * 
 * @param obj 连接体
 * @param buf 发送缓冲区
 * @param buf_size 缓冲区大小
 * @param timeout 超时
 * 
 * @return 成功,返回发送数据字节数;失败,返回-1
 */
int sw_httpserver_send_response_content(http_connect_obj_t* obj,
											char*buf, int buf_size,
											int timeout);

/** 
 * @brief 关闭连接,同时释放资源
 * 
 * @param obj 连接体 
 */
void sw_httpserver_close_connectobj(http_connect_obj_t* obj);
	
#ifdef __cplusplus
}
#endif

#endif
