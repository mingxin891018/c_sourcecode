/**
* @file swudp.c
* @brief UDP函数接口
* @author huanghuaming
* @history
*           2004-04-28 huanghuaming created
*           2010-07-07 qushihui modified
*/

#include "swapi.h"
#include "swos_priv.h"
#include "swudp.h"

/** 
 * @brief 创建UDP套接字
 * 
 * @return 成功,返回创建的套接字; 否则,返回-1
 */
int sw_udp_socket()
{
	return socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
}

/** 
 * @brief 关闭套接字
 * 
 * @param skt 创建的套接字
 */
void sw_udp_close( int skt )
{
	if( skt != -1 )
	{
		close( skt );
	}

	return;
}

/** 
 * @brief 绑定本地地址和端口
 * 
 * @param skt UDP套接字
 * @param ip 本地的ip地址(网络字节序)
 * @param port 本地端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_bind( int skt, unsigned long ip, unsigned short port )
{
	struct sockaddr_in local;

	/* 绑定本地地址和端口 */
	memset( &local, 0, sizeof(local) );
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = ip;
	local.sin_port = port;

	if( bind ( skt, (struct sockaddr *)&local,	sizeof(local) ) < 0 )
	{
		OS_LOG_DEBUG( "cannot bind\n" );
		return -1;
	}

	return 0;
}

/** 
 * @brief 加入组播组
 * 
 * @param skt 套接字
 * @param ip 加入组播的ip地址(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_join( int skt, unsigned long ip )
{
	int ttl = 127;
	bool flag = true;
	struct ip_mreq mreq;

	/* 加入多播组 */
	if( IS_MULTICAST_IP(ip) )
	{
		memset( &mreq, 0, sizeof(mreq) );
		mreq.imr_multiaddr.s_addr = ip;
		mreq.imr_interface.s_addr = INADDR_ANY;

 		if( setsockopt( skt, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) < 0 )
		{
			OS_LOG_DEBUG( "cannot add to multicast\n" );
			return -1;
		}
		if( setsockopt( skt, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast args. \n" );
		}
		if( setsockopt( skt, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&flag, sizeof(flag) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast ttl args. \n" );
		}
	}
	else
	{
		OS_LOG_DEBUG("not a multicast ip\n");
		return -1;
	}

	return 0;
}

/** 
 * @brief 退出组播组
 * 
 * @param skt 套接字
 * @param ip 退出组播组的ip地址
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_drop( int skt, unsigned long ip )
{
	/* 如果监听地址在组播地址范围内，则退出接收组播 */
	if( IS_MULTICAST_IP(ip) )
	{
		struct ip_mreq mreq;

		memset( &mreq, 0, sizeof(mreq) );
		mreq.imr_multiaddr.s_addr = ip;
		mreq.imr_interface.s_addr = INADDR_ANY;
		if( setsockopt( skt, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) <0 )
		{
			OS_LOG_DEBUG( "cannot drop from multicast\n" );
			return -1;
		}
	}

	return  0;
}

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
int sw_udp_send( int skt, unsigned long ip, unsigned short port, char *buf, int size )
{
	struct sockaddr_in sn;
	unsigned int slen = sizeof(sn);

	memset( &sn, 0, sizeof(sn) );
	slen = sizeof(sn);
	sn.sin_family = AF_INET;
	sn.sin_addr.s_addr = ip;
	sn.sin_port=  port;

	return sendto( skt, buf, size, 0, (struct sockaddr *)&sn, slen );
}

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
int sw_udp_recv( int skt, unsigned long *ip, unsigned short *port, char *buf, int size )
{
	struct sockaddr_in from;
	int slen = sizeof( from );

	memset( &from, 0, sizeof(from) );
	slen = recvfrom( skt, buf, size, 0, (struct sockaddr *)&from, &slen );

	if( ip )
	{
	   	*ip = from.sin_addr.s_addr;
	}
	if( port )
	{
		*port = from.sin_port;
	}

	return slen;
}

/** 
 * @brief 配置套接字的类型
 * 
 * @param skt 套接字
 * @param type 套接字的类型
 * @param val 指向的数值,通常取值范围为(0,1)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_ioctl( int skt, int type, unsigned long *val )
{
	return ioctl( skt, (long)type, val );
}

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
int sw_udp_select( int skt, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, int timeout )
{
	struct timeval tv;

	if( readfds )
	{
		FD_ZERO( readfds );
		FD_SET( (unsigned int)skt, readfds );
	}
	if( writefds )
	{
		FD_ZERO( writefds );
		FD_SET( (unsigned int)skt, writefds );
	}
	if( exceptfds )
	{
		FD_ZERO( exceptfds );
		FD_SET( (unsigned int)skt, exceptfds );
	}
	if( 0 <= timeout )
	{
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout%1000)*1000;

		return select( skt+1, readfds, writefds, exceptfds, &tv );
	}
	else
	{
		return select( skt+1, readfds, writefds, exceptfds, NULL );
	}
}


/** 
 * @brief 创建UDP套接字
 * 
 * @return 成功,返回创建的套接字; 否则,返回-1
 */
int sw_udp_socket_v6()
{
	return socket( AF_INET6, SOCK_DGRAM, IPPROTO_UDP );
}

/** 
 * @brief 绑定本地地址和端口
 * 
 * @param skt UDP套接字
 * @param ip 本地的ip地址(网络字节序)
 * @param port 本地端口(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_bind_v6( int skt, struct in6_addr ip, unsigned short port )
{
	struct sockaddr_in6 local;

	/* 绑定本地地址和端口 */
	memset( &local, 0, sizeof(local) );
	local.sin6_family = AF_INET6;
	local.sin6_addr = ip;
	local.sin6_port = port;

	if( bind ( skt, (struct sockaddr *)&local,	sizeof(local) ) < 0 )
	{
		OS_LOG_DEBUG( "cannot bind\n" );
		return -1;
	}

	return 0;
}

/** 
 * @brief 加入组播组
 * 
 * @param skt 套接字
 * @param ip 加入组播的ip地址(网络字节序)
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_join_v6( int skt, struct in6_addr ip )
{
	int ttl = 127;
	bool flag = true;
	struct ipv6_mreq mreq;

	/* 加入多播组 */
	if( IS_MULTICAST_IPV6(ip) )
	{
		memset( &mreq, 0, sizeof(mreq) );
		mreq.ipv6mr_multiaddr = ip;
		mreq.ipv6mr_interface = 0;

 		if( setsockopt( skt, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) < 0 )
		{
			OS_LOG_DEBUG( "cannot add to multicast\n" );
			return -1;
		}
		if( setsockopt( skt, IPPROTO_IPV6, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast args. \n" );
		}
		if( setsockopt( skt, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (char *)&flag, sizeof(flag) ) == SOCKET_ERROR )
		{
			OS_LOG_DEBUG( "cannot set multicast ttl args. \n" );
		}
	}
	else
	{
		OS_LOG_DEBUG("not a multicast ip\n");
		return -1;
	}

	return 0;
}

/** 
 * @brief 退出组播组
 * 
 * @param skt 套接字
 * @param ip 退出组播组的ip地址
 * 
 * @return 失败,返回负数(小于0); 成功,返回其他值
 */
int sw_udp_drop_v6( int skt, struct in6_addr ip )
{
	/* 如果监听地址在组播地址范围内，则退出接收组播 */
	if( IS_MULTICAST_IPV6(ip) )
	{
		struct ipv6_mreq mreq;

		memset( &mreq, 0, sizeof(mreq) );
		mreq.ipv6mr_multiaddr = ip;
		mreq.ipv6mr_interface = 0;
		if( setsockopt( skt, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq) ) <0 )
		{
			OS_LOG_DEBUG( "cannot drop from multicast\n" );
			return -1;
		}
	}

	return  0;
}

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
int sw_udp_send_v6( int skt, struct in6_addr ip, unsigned short port, char *buf, int size )
{
	struct sockaddr_in6 sn;
	unsigned int slen = sizeof(sn);

	memset( &sn, 0, sizeof(sn) );
	slen = sizeof(sn);
	sn.sin6_family = AF_INET6;
	sn.sin6_addr = ip;
	sn.sin6_port=  port;

	return sendto( skt, buf, size, 0, (struct sockaddr *)&sn, slen );
}

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
int sw_udp_recv_v6( int skt, struct in6_addr *ip, unsigned short *port, char *buf, int size )
{
	struct sockaddr_in6 from;
	int slen = sizeof( from );

	memset( &from, 0, sizeof(from) );
	slen = recvfrom( skt, buf, size, 0, (struct sockaddr *)&from, &slen );

	if( ip )
	{
	   	*ip = from.sin6_addr;
	}
	if( port )
	{
		*port = from.sin6_port;
	}

	return slen;
}

