/**
 * @file   swhttpsserver.h
 * @brief  http server
 * @version %I% %G%
 * @author Hongchen Dou
 * @history 
 * 				2007-07-15 Hongchen Dou created
 * 				2010-07-29 qushihui modified 
 */

#ifndef __SWHTTPSSERVER_H__
#define __SWHTTPSSERVER_H__


#include "swhttpserver.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * 开启一个httpserver 
 * 
 * @param port 服务器端口号， 网络序
 * @param callback 回调函数
 * @param wparam  回调函数的参数
 * 
 * @return 返回Httpserver句柄
 */
HANDLE sw_httpsserver_open(unsigned short port, http_server_callback callback, uint32_t wparam, const char *certpath, const char *keypath);
HANDLE sw_httpsserver_open_v6(unsigned short port, http_server_callback callback, uint32_t wparam );


/** 
 * 关闭httpserver
 * 
 * @param server 
 */
void sw_httpsserver_close( HANDLE server );

/** 
 * 接收Request报头， 成功则填充request_header结构
 * 
 * @param obj 
 * @param timeout 
 * 
 * @return int: 成功返回报文头的大小， 失败返回负数
 */
int sw_httpsserver_recv_request_header(HANDLE sobj, int timeout );

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
int sw_httpsserver_recv_request_content(HANDLE sobj, char*buf, int buf_size, int timeout );


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
int sw_httpsserver_send_response_header(HANDLE sobj, http_response_num_t responseNum, 
									   char* pContentType, char* pContentEncoding, 
									   char* pConnection,
									   int nContentLength, 
									   int timeout);
	
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
int sw_httpsserver_send_response_content(HANDLE sobj,
											char*buf, int buf_size,
											int timeout);

/** 
 * 关闭连接,同时释放资源
 * 
 * @param obj 
 */
void sw_httpsserver_close_connectobj(HANDLE sobj);
	
#ifdef __cplusplus
}
#endif

#endif
