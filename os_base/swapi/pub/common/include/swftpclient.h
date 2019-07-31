/** 
 * @file swftpclient.h
 * @brief FTP函数接口
 * @author ...
 * @date 2007-09-06
 * @history
 * @2010-8-5 xusheng modify the format
 */


#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

typedef struct
{
	/* ftp命令服务器工作套接字 */
	int cmdskt;
	/* ftp数据服务器工作套接字 */
	int datskt;
	/* 工作类型	 */
	int type;
	/* 服务器ip */
	unsigned long ip;
	/* 服务器命令端口 */
	unsigned short cmdport;
	/* 服务器数据端口 */
	unsigned short datport;
	/* ftp命令服务器回复 */
	char response[512];
}sftpclient_t;

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief ：与服务器建立连接
 * 
 * @param ip 服务器的ip (ip和port均为网络字节序)
 * @param port 连接端口
 * @param timeout 超时时间
 * 
 * @return 工作句柄
 */
HANDLE sw_ftpclient_connect( unsigned long ip, unsigned short port, int timeout );

/** 
 * @brief 断开与ftp服务器的连接
 * 
 * @param hftp 工作句柄
 * @param timeout 超时时间
 */
void sw_ftpclient_disconnect( HANDLE hftp, int timeout );

/** 
 * @brief 登入ftp服务器,建立数据端口连接
 * 
 * @param hftp 工作句柄
 * @param username 用户名
 * @param password 密码
 * @param timeout 超时时间
 * 
 * @return true,成功;否则, false
 */
bool sw_ftpclient_login( HANDLE hftp, char* username, char* password, int timeout );

/** 
 * @brief 从ftp服务器数据端口接收数据
 * 
 * @param hftp 工作句柄
 * @param buf 接收缓冲
 * @param size 缓冲大小
 * @param timeout 超时时间
 * 
 * @return 接收大小
 */
int sw_ftpclient_recv_data( HANDLE hftp, char *buf, int size, int timeout );

/** 
 * @brief 向ftp服务器数据端口发送数据
 * 
 * @param hftp 工作句柄
 * @param buf 发送缓冲
 * @param size 缓冲大小
 * @param timeout 超时时间
 * 
 * @return 发送大小
 */
int sw_ftpclient_send_data( HANDLE hftp, char *buf, int size, int timeout );

/** 
 * @brief 向ftp服务器发送命令
 * 
 * @param hftp 工作句柄
 * @param type 命令类型
 * @param command 命令
 * @param timeout 超时时间
 * 
 * @return 服务器的回复
 */
char* sw_ftpclient_send_command( HANDLE hftp, char *type, char* command, int timeout );


#ifdef __cplusplus
}
#endif
#endif /* _FTP_CLIENT_H_ */
