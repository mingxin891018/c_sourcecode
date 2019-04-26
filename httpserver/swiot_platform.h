#ifndef __SWIOT_PLATFORM_H__
#define __SWIOT_PLATFORM_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <pthread.h>
#include <errno.h>

#include "swiot_common.h"

#define SWIOT_Malloc   malloc
#define SWIOT_Memset   memset
#define SWIOT_Free     free
#define SWIOT_htons(x) htons(x)

#define SWIOT_Thread_Create( handle,callTask,threadName,stack_size,parameters, priority ) ((0==pthread_create(&handle, NULL, callTask, parameters))?1:0)
#define SWIOT_Thread_Destroy(handle) (pthread_join(handle,NULL))

#define SWIOT_msleep(time)    usleep((time*1000))

#define SWIOT_Get_Time()    time(NULL)
#define SWIOT_Inet_Addr     inet_addr
#define SWIOT_Select        select
#define SWIOT_GethostByName gethostbyname

void* SWIOT_Mutex_Create();
void SWIOT_Mutex_Destroy( void* mutex );
void SWIOT_Mutex_Lock( void* mutex );
void SWIOT_Mutex_Unlock( void* mutex );

/**
 * [创建套接字]
 * @param  domain   [协议族，常用协议族有AF_INET（要使用ipv4地址）、AF_INET6、AF_LOCAL ]
 * @param  type     [指定Socket类型,常用的SOCKET类型有SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET]
 * @param  protocol [指定协议，常用协议有IPPROTO_TCP,IPPROTO_UDP,IPPROTO_STCP,IPPROTO_TIPC，与type不可随意组合]
 * @return          [成功返回套接字，否则 < 0]
 */
int SWIOT_Creat_Socket(int domain,int type,int protocol);

/**
 * [销毁套接字]
 * @param  skt [套接字]
 * @return     [错误 < 0]
 */
int SWIOT_Close_Socket(int skt);

/**
 * [绑定套接字]
 * @param  skt  [需要绑定的套接字]
 * @param  ip   [绑定的Ip地址]
 * @param  port [绑定的端口]
 * @return      [成功返回0]
 */
int SWIOT_Bind_Socket(int skt,unsigned int ip,unsigned short port);

/**
 * [设置套接字选项]
 * @param  skt        [套接字]
 * @param  level      [所在协议层]
 * @param  optname    [需要访问的选项名]
 * @param  optval     [选项值的缓冲]
 * @param  optval_len [选项的长度]
 * @return            [失败<0]
 */
int SWIOT_Setsocket_Opt(int skt,int level,int optname,void* optval,int optval_len);


/**
 * [建立连接]
 * @param  skt  [套接字]
 * @param  ip   [连接ip]
 * @param  port [连接port]
 * @return      [失败 < 0 如果为非阻塞 且errno == EINPROGRESS 表示连接正在建立中]
 */
int SWIOT_Connect( int skt,unsigned int ip,unsigned short port );


int SWIOT_Fcntl(int s, int cmd, int val);

/**
 * [接受连接]
 * @param  skt  [套接字]
 * @param  ip   [对端ip]
 * @param  port [对端port]
 * @return      [<0 返回失败]
 */
int SWIOT_Accept( int skt,unsigned int *ip,unsigned short *port );


/**
 * [接受数据]
 * @param  skt [接受的套接字]
 * @param  buf [接受缓存区]
 * @param  len [缓存区大小]
 * @return     [== 0 表示对方连接中断 <0 且 errno == EAGAIN(栈满或空) || errno == EINTR(表示系统调用被中断) 表示可以继续读 其余为异常]
 */
int SWIOT_Recv( int skt,char *buf,int len );

/**
 * [发送数据]
 * @param  skt [发送的套接字]
 * @param  buf [发送缓存区]
 * @param  len [缓存区长度]
 * @return     [== 0 表示对方连接中断 <0 且 errno == EAGAIN(栈满或空) || errno == EINTR(表示系统调用被中断) 表示可以继续写 其余为异常]
 */
int SWIOT_Send( int skt,char *buf,int len );


/**
 * [接收一个数据报，并保存数据报源地址]
 * @param  skt       [套接口]
 * @param  buf       [接收缓存区]
 * @param  len       [缓存区大小]
 * @param  from_ip   [源地址ip]
 * @param  from_port [源地址port]
 * @return           []
 */
int SWIOT_Recv_From(int skt,char* buf,int len,unsigned int* from_ip,unsigned short*from_port);

/**
 * [发送数据报]
 * @param  skt     [套接口]
 * @param  buf     [发送缓存区]
 * @param  len     [缓存区大小]
 * @param  to_ip   [目的端ip]
 * @param  to_port [目的端port]
 * @return         [description]
 */
int SWIOT_Send_To(int skt,char* buf,int len,unsigned int to_ip,unsigned short to_port);


#endif //__SWIOT_PLATFORM_H__
