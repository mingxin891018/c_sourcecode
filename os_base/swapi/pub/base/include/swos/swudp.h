/** 
 * @file swudp.h
 * @brief UDP函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */

#ifndef __SWUDP_H__
#define __SWUDP_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "swapi.h"
/** 
 * @brief 创建UDP套接字
 * 
 * @return 成功,返回创建的套接字; 否则,返回-1
 */
int sw_udp_socket();
int sw_udp_socket_v6();

/** 
 * @brief 关闭套接字
 * 
 * @param skt 创建的套接字
 */
void sw_udp_close( int skt );

/** 
 * @brief 绑定本地地址和端口
 * 
 * @param skt UDP套接字
 * @param ip 本地的ip地址(网络字节序)
 * @param port 本地端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_bind( int skt, unsigned long ip, unsigned short port );
int sw_udp_bind_v6( int skt, struct in6_addr ip, unsigned short port );

/** 
 * @brief 加入组播组
 * 
 * @param skt 套接字
 * @param ip 加入组播的ip地址(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_join( int skt, unsigned long ip );
int sw_udp_join_v6( int skt, struct in6_addr ip );

/** 
 * @brief 退出组播组
 * 
 * @param skt 套接字
 * @param ip 退出组播组的ip地址
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_drop( int skt, unsigned long ip );
int sw_udp_drop_v6( int skt, struct in6_addr ip );

/** 
 * @brief 发送数据
 * 
 * @param skt 套接字
 * @param ip 远端的ip地址(网络字节序)
 * @param port 远端的端口(网络字节序)
 * @param buf 发送数据的缓冲区
 * @param size 发送数据缓冲区的大小
 * 
 * @return 成功,返回实际发送的字节数; 否则,返回-1
 */
int sw_udp_send( int skt, unsigned long ip, unsigned short port, char *buf, int size );
int sw_udp_send_v6( int skt, struct in6_addr ip, unsigned short port, char *buf, int size );

/** 
 * @brief 接收数据
 * 
 * @param skt 套接字
 * @param ip 接收到的远端ip
 * @param port 接收到的远端端口
 * @param buf 接收数据的缓冲区
 * @param size 接收数据缓冲区的大小
 * 
 * @return 成功,返回实际接收的字节数; 否则,返回-1
 */
int sw_udp_recv( int skt, unsigned long *ip, unsigned short *port, char *buf, int size );
int sw_udp_recv_v6( int skt, struct in6_addr *ip, unsigned short *port, char *buf, int size );

/** 
 * @brief 配置套接字的类型
 * 
 * @param skt 套接字
 * @param type 套接字的类型
 * @param val 指向的数值,通常取值范围为(0,1)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_ioctl( int skt, int type, unsigned long *val );

/** 
 * @brief 检测套接字状态的变化
 * 
 * @param skt 套接字
 * @param readfds 读描述词组
 * @param writefds 写描述词组
 * @param exceptfds 例外描述词组
 * @param timeout 等待的超时时间(ms)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_select( int skt, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, int timeout );


#ifdef __cplusplus
}
#endif

#endif /*__SWUDP_H__*/
