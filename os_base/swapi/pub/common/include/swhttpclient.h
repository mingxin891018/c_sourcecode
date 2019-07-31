/** 
 * @file swhttpclient.h
 * @brief HTTP函数接口
 * @version %I% %G%
 * @author NiuJiuRu
 * @date 2007-10-29
 * @history
 * 			2010-07-29	qushihui modified
 */

#ifndef __SWHTTPCLIENT_H__
#define __SWHTTPCLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif



#define HTTP_OK 0
#define HTTP_ERROR_PORT_NORESPONSE 1
#define HTTP_ERROR_PORT_DENY  2 
#define HTTP_ERROR_HTTP_NORESPONSE 3
#define HTTP_ERROR_TIMEOUT 4
#define HTTP_ERROR_EXCEPTION 5
#define HTTP_ERROR_HTTP_NORESPONSE_404 404
#define HTTP_ERROR_HTTP_NORESPONSE_302 302
#define HTTP_ERROR_HTTP_NORESPONSE_403 403
#define HTTP_ERROR_HTTP_NORESPONSE_500 500
#define HTTP_ERROR_HTTP_NORESPONSE_206 206
#define HTTP_ERROR_HTTP_NORESPONSE_301 301
#define HTTP_ERROR_HTTP_NORESPONSE_400 400
#define HTTP_ERROR_HTTP_NORESPONSE_406 406
#define HTTP_ERROR_HTTP_NORESPONSE_503 503


//HTTP摘要认证
typedef struct http_authentication
{
	char arithmetic[10];		// 算法名称: md5, md5-sess
	char user_name[64];		// 用户名 
	char user_pwd[64];		// 密码
	char realm[128];			// realm name
	char nonce[48];			// 服务器随机产生的nonce返回串
	char uri[256];			// 请求URL
	char qop[10];			// qop-value: "", "auth", "auth-int"
	char opaque[48];		// opaque value
}http_authentcation_t;


HANDLE sw_httpclient_init(unsigned long  ip, unsigned short port);
bool sw_httpclient_connect2( HANDLE h_http_client, int timeout);

/** 
 * @brief 与Http服务器建立连接
 * 
 * @param ip http服务器地址
 * @param port http服务器端口
 * @param timeout 
 * 
 * @return 成功，返回句柄；失败，返回NULL
 */
HANDLE sw_httpclient_connect( unsigned long ip, unsigned short port, int timeout );
HANDLE sw_httpclient_connect_v6( struct in6_addr ip, unsigned short port, int timeout );
HANDLE sw_httpclient_connect_ext( unsigned long  ip, unsigned short port, int timeout, bool is_https);

/** 
 * @brief 断开与Http服务器建立的连接
 * 
 * @param h_http_client 连接成功后的句柄
 */
void sw_httpclient_disconnect( HANDLE h_http_client );

void sw_httpclient_shutdown ( HANDLE h_http_client);
void sw_httpclient_close( HANDLE h_http_client);

/** 
 * @brief 发送HTTP Request
 * 
 * @param h_http_client 建立连接后的句柄
 * @param method 请求方式
 * @param url 请求的URL
 * @param host 主机名称
 * @param accept_type 接收的文件类型
 * @param content_type 内容类型
 * @param content 内容
 * @param length 内容长度
 * @param timeout 超时
 * @param soap_action: soap action URI
 * @param http_auth 摘要认证(NULL, 不使用摘要认证; NOT NULL, 要使用的摘要认证信息)
 * 
 * @return 请求发送成功,true;失败,false
 */
/*
bool sw_httpclient_request( HANDLE h_http_client, char* method, char* url, char *host,
							char *accept_type, char* content_type, char* content, int length, 
							int timeout, char* soap_action, SHttpAuthentcation* http_auth );  */

bool sw_httpclient_request( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth );

/** 
 * @brief 发送HTTP Request TTNET request
 * 
 * @param h_http_client 建立连接后的句柄
 * @param method 请求方式
 * @param url 请求的URL
 * @param host 主机名称
 * @param accept_type 接收的文件类型
 * @param content_type 内容类型
 * @param content 内容
 * @param length 内容长度
 * @param timeout 超时
 * @param soap_action: soap action URI
 * @param http_auth 摘要认证(NULL, 不使用摘要认证; NOT NULL, 要使用的摘要认证信息)
 * 
 * @return 请求发送成功,true;失败,false
 */
bool sw_httpclient_request_ex( HANDLE h_http_client, char* method, char* url, char* host,
							char *accept_type, char* content_type, char* content, int length,
							int timeout, char* soap_action, http_authentcation_t* http_auth );	

//[in]filename:要上传文件的文件名
bool sw_httpclient_request_ex2( HANDLE h_http_client, char* method, char* url, char* host,
		                    char *accept_type,char *filename, char* content_type, char* content, int length,
		                    int timeout, char* soap_action, http_authentcation_t* http_auth );


/** 
 * @brief 接收HTTP数据
 * 
 * @param h_http_client 建立连接后的句柄
 * @param buf 接收缓冲区
 * @param size 缓冲区大小
 * @param timeout 接收超时
 * 
 * @return 成功，返回接收到的数据长度；失败，返回-1
 */
int sw_httpclient_recv_data( HANDLE h_http_client, char* buf, int size, int timeout );

/**
 * @brief 接收HTTP的相应头数据
 * @param h_http_client 建立连接后的套接字
 * @param buf 接收响应头的缓冲区
 * @param size 接收响应头的缓冲区数据最大长度
 * @param timeout 接收超时
 * 
 * @return 接收响应头的数据长度(如果缓冲区长度不够有可能没收完响应头域)
*/
int sw_httpclient_recv_header( HANDLE h_http_client, char* buf, int size, int timeout );

/** 
 * @brief 取得Http响应头域的属性字段
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param bufsize HTTP报文头缓冲区大小
 * @param name 头域属性名需要带上':'号
 * @param namelen 头域属性名长度
 * @param value 头域属性值缓冲区-----如果为空地话使用m_tmp_field(多http链接时会不准确)
 * @param valuesize 头域属性值缓冲区长度
 * 
 * @return 
 */
char* sw_httpclient_get_field_value(char* head_buf, int bufsize, char *name, int namelen, char *value, int valuesize);

/** 
 * @brief 取得负载长度"Content-Length:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 成功返回负载长度，失败返回-1
 */
int64_t sw_httpclient_get_content_size( char* head_buf, int size );

/** 
 * @brief 取得返回值
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 成功，返回返回值大小；失败，返回-1
 */
int sw_httpclient_get_ret_code( char* head_buf, int size );

/** 
 * @brief 得到http header的长度
 * 
 * @param head_buf HTTP报文头接收
 * @param size 缓冲区大小
 * 
 * @return 成功，返回http header的长度；失败，返回-1
 */
int sw_httpclient_get_header_size( char* head_buf, int size );

/** 
 * @brief 注册cookies
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param cookies 预设置的cookies存放缓冲区
 * 
 */
//原来是  void sw_httpclient_register_cookies( HANDLE h_http_client, char* cookies );
int sw_httpclient_register_cookies( HANDLE h_http_client, char* cookies );

/** 
 * @brief 清空cookies
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 */
void sw_httpclient_clear_cookies( HANDLE h_http_client );

/** 
 * @brief 取得cookies
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 * @return h_http_client cookie 存储区
 */
char* sw_httpclient_get_cookies( HANDLE h_http_client );

/** 
 * @brief 注册User Agent
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param useragent 预设置的User Agent存放缓冲区,格式User-Agent: 
 * 
 * @return 
 */
int sw_httpclient_register_useragent( HANDLE h_http_client, const char* useragent );

/** 
 * @brief 清空User Agent
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 * @return
 */
void sw_httpclient_clear_useragent( HANDLE h_http_client );

/** 
 * @brief 取得User Agent
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 * @return h_http_client cookie 存储区
 */
char* sw_httpclient_get_useragent( HANDLE h_http_client );

/** 
 * @brief 设置HTTP版本号
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param version 预设置的HTTP版本号
 * 
 */
void sw_httpclient_set_version( HANDLE h_http_client, char* version );

/** 
 * @brief  取得HTTP client 的socket
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 * @return  成功,返回http client连接的socket;失败,返回-1
 */
int sw_httpclient_get_skt( HANDLE h_http_client );

int sw_httpclient_get_err_code(HANDLE h_http_client);

void sw_httpclient_set_err_code(HANDLE h_http_client, int code);

unsigned short sw_httpclient_get_listen_port( );

/** 
 * @brief 判断,取得负载长度"Content-Length:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return true存在content-length域，false不存在
 */
bool sw_httpclient_check_content_size( char* head_buf, int size, int64_t *contentsize );

/** 
 * @brief 取得负载文件的长度范围"Content-Range:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	startpos http报文头指示的文件片段起始位值
 *	endpos http报文头指示的文件片段结束位置(如果有Content-Length,其：endpos-startpos+1 = Content-Length)
 *	filesize http报文头指示的文件总大小
 * @return 是否含有range字段
 */
bool sw_httpclient_get_content_range( char* head_buf, int size,  int64_t *startpos, int64_t *endpos, int64_t *filesize);

/** 
 * @brief 取得负载文件的"Content-MD5:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	value md5缓冲区,如果value为空返回的句柄需要立即使用或者有可能别改变
 *	valuesize缓冲区长度
 * @return 非空存在md5校验,NULL不存在
 */
char* sw_httpclient_get_content_md5( char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief 取得负载文件的编码方式"Content-Encoding:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	value encoding缓冲区,如果value为空返回的句柄需要立即使用或者有可能别改变
 *	valuesize缓冲区长度
 * @return 非空存在编码，NULL不存在编码字段,使用非压缩文件传输
 */
char* sw_httpclient_get_content_encoding(char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief 设置HTTP连接可接受的编码方式
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param encoding 预设置的HTTP编码方式
 * 
 * @return 
 */
void sw_httclient_set_encoding(HANDLE h_http_client, char* encoding );

/** 
 * @brief 设置HTTP连接可接受的语言
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param language 预设置的HTTP语言
 * 
 * @return 
 */
void sw_httclient_set_language(HANDLE h_http_client, const char* language );

/** 
 * @brief 设置HTTP链接的If-None-Match
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param etag	ETag号
 * 
 * @return 
 */
void sw_httpclient_set_etag(HANDLE h_http_client, char* etag );

/** 
 * @brief 取得负载文件的HTTP ETag
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	value type缓冲区,如果value为空返回的句柄需要立即使用或者有可能别改变
 *	valuesize 缓冲区长度
 * @return 非空存在编码，NULL不存在编码字段,使用非压缩文件传输
 */
char* sw_httpclient_get_etag(char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief 取得负载文件的传输译码"Transfer-Encoding:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	value Transfer Encoding缓冲区,如果value为空返回的句柄需要立即使用或者有可能别改变
 *	valuesize缓冲区长度
 * @return 非空存在传输译码，NULL不存在
 */
 char* sw_httpclient_get_transfer_encoding(char* head_buf, int size,  char *value, int valuesize);
 
/** 
 * @brief 取得负载文件的"Location:"重定向
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	value location缓冲区,如果value为空返回的句柄需要立即使用或者有可能别改变
 *	valuesize缓冲区长度
 * @return 非空存在重定向，NULL不存在重定向
 */
 char* sw_httpclient_get_location(char* head_buf, int size,  char *value, int valuesize);
 
/** 
 * @brief 取得负载文件的关联的"Set-Cookie:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	value 关联的Cookie缓冲区,如果value为空返回的句柄需要立即使用或者有可能别改变
 *	valuesize缓冲区长度
 * @return 非空存在关联Cookie，NULL不存在
 */
char* sw_httpclient_get_set_cookie(char* head_buf, int size,  char *value, int valuesize);

/** 
 * @brief 判断服务器是否支持byte-range requests"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return true支持byte-range requests，false不支持
 */
bool sw_httpclient_check_accept_ranges(char* head_buf, int size);

/** 
 * @brief 设置HTTP链接的私有字段(百事通)
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param extra 私有字段
 * 
 * @return 
 */
void sw_httpclient_set_extra(HANDLE h_http_client, char* extra);

/** 
 * @brief 设置HTTP链接的私有字段(百事通)
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param extra 私有字段
 * 
 * @return 
 */
void sw_httpclient_set_getrange(HANDLE h_http_client, const char* getrange);

/** 
 * @brief 取得负载文件的播放范围"PlayTime-Range:"
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 	start http报文头指示的文件片段起始时间
 *	end http报文头指示的文件片段结束时间
 *	duration报文头指示的文件可供播放时间
 * @return 是否含有PlayTime-Range字段
 */
bool sw_httpclient_get_playtime_range( char* head_buf, int size,  int64_t *start, int64_t *end, int64_t *duration);

/** 
 * @brief 设置HTTP请求头域字段那些不需要发送
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param ex_field 头域字段名
 *								Accept-language
 *								Accept-Encoding
 *								User-Agent
 *								Pragma
 *								Cache-Control
 *								Cookie
 * 
 * @return 
 */
void sw_httpclient_set_exclude_field(HANDLE h_http_client, const char* ex_field);

#ifdef __cplusplus
}
#endif

#endif /* __SWHTTPCLIENT_H__ */

