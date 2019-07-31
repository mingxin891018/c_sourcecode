/** 
 * @file swhttpsclient.h
 * @brief HTTP函数接口
 * @author NiuJiuRu
 * @date 2007-10-29
 * @history
 * 				2007-10-29 NiuJiuRu created
 * 				2010-07-29 qushihui modified
 */

#ifndef __SWHTTPSCLIENT_H__
#define __SWHTTPSCLIENT_H__

#include "swhttpclient.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 与Http服务器建立连接,如果证书路径存在或缓存非空请确保证书正确
 * 
 * @param ip http服务器地址
 * @param port http服务器端口
 * @param timeout 
 * @param certpath证书路径,sw_httpsclient_connect这个使用默认的证书路径
 * @param certbuf证书缓存----这个不为空优先使用这个而不是certpath
 * @param certlen证书缓存数据长度,如果certbuf不为空len<=0时使用strlen判断
 * 
 * @return 返回连接成功后的句柄
 */
HANDLE sw_httpsclient_connect_verify( unsigned long  ip, unsigned short port, int timeout, void *key );

HANDLE sw_httpsclient_connect( unsigned long ip, unsigned short port, int timeout);
//在secure分支有实现
HANDLE sw_httpsclient_connect_ex( unsigned long ip, unsigned short port, int timeout, const char *certpath, const char *certbuf, int certlen);
HANDLE sw_httpsclient_connect_v6( struct in6_addr ipv6, unsigned short port, int timeout, const char *certpath, const char *certbuf, int certlen);

/************************secure分支下面的函数都可以使用httpclient里面的对应接口,只有在建立链接时需要指明**********************************************/
/** 
 * @brief 断开与Http服务器建立的连接
 * 
 * @param h_http_client 连接成功后的句柄
 */
void sw_httpsclient_disconnect( HANDLE h_http_client );

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
 * @return 请求是否发送成功
 */
bool sw_httpsclient_request( HANDLE h_http_client, char* method, char* url, char *host, char *accept_type, char* content_type,
	   							char* content, int length, int timeout, char* soap_action, http_authentcation_t* http_auth );
/*
[in] pFileName:要上传的文件名
*/
bool sw_httpsclient_request_ex( HANDLE hHttpclient, char* pMethod,char* pURL,char *pHost, char *pAcceptType,char *pFileName,
		                        char* pContentType,char* pContent,int nLength, int timeout,char* pSOAPAction,http_authentcation_t* pHttpAuth);
/** 
 * @brief 接收HTTP数据
 * 
 * @param h_http_client 建立连接后的句柄
 * @param buf 接收缓冲区
 * @param size 缓冲区大小
 * @param timeout 接收超时
 * 
 * @return 接收到的数据长度
 */
int sw_httpsclient_recv_data( HANDLE h_http_client, char* buf, int size, int timeout );

/** 
 * @brief 取得负载长度
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int64_t sw_httpsclient_get_content_size( char* head_buf, int size );

/** 
 * @brief 取得返回值
 * 
 * @param head_buf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpsclient_get_ret_code( char* head_buf, int size );

/** 
 * @brief 得到http header的长度
 * 
 * @param head_buf HTTP报文头接收
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpsclient_get_header_size( char* head_buf, int size );

/** 
 * @brief 注册cookies
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param cookies 预设置的cookies存放缓冲区
 * 
 * @return 
 */
int sw_httpsclient_register_cookies( HANDLE h_http_client, char* cookies );

/** 
 * @brief 清空cookies
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 * @return
 */
void sw_httpsclient_clear_cookies( HANDLE h_http_client );

/** 
 * @brief 取得cookies
 * 
 * @param h_http_client 已连接客户端的句柄
 * 
 * @return h_http_client cookie 存储区
 */
char* sw_httpsclient_get_cookies( HANDLE h_http_client );

/** 
 * @brief 设置HTTP版本号
 * 
 * @param h_http_client 已连接客户端的句柄
 * @param version 预设置的HTTP版本号
 * 
 * @return 
 */
void sw_httpsclient_set_version( HANDLE h_http_client, char* version );


#ifdef __cplusplus
}
#endif

#endif /* __SWHTTPCLIENT_H__ */

