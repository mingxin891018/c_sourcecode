/**
 * @file test_tcp.c
 * @brief test tcp.c
 * @author qushihui
 * @date 2010-07-05
 * @history
 *		2010-07-19 created
 */
#include "swtcp.h"
#include "swos.h"
#include "swlog.h"
#include "skeleton.h"

static unsigned long  m_ip;				//服务器ip 
static unsigned short m_port;			//服务器端口
static char m_file[24];					//要传输的文件名

/** 
 * @brief 测试服务器端
 *   
 * @return 成功返回0;失败返回-1
 */   
int test_tcp_server( void )
{
	int ser_skt, lis_skt;
	char recv_buf[1024];
	int fd;
	int recv_size;
	unsigned long unblock = 1;
	fd_set recv_fds;

	//创建套接字
	if( ( lis_skt = sw_tcp_socket() ) < 0 )
	{
		test_log( FAIL , "A fail case test. fail to open socket\n" );
		return -1;
	}
	OS_LOG_DEBUG( "lis_skt=%d\n", lis_skt );
	
	//设置套接字为非阻塞
	if( sw_tcp_ioctl( lis_skt, FIONBIO, &unblock ) < 0 )
	{
		test_log( FAIL , "A fail case test. fail to  set unblock\n" );
		return -1;
	}
	OS_LOG_DEBUG( "set unblock\n" );
	
	//绑定
	if( sw_tcp_bind( lis_skt, m_ip, m_port ) < 0)
	{
		test_log( FAIL , "A fail case test. fail to  bind\n" );
		return -1;
	}
	OS_LOG_DEBUG( "bind the server socket\n" );

	//监听
	if( sw_tcp_listen( lis_skt, 5 ) < 0 )
	{
		test_log( FAIL , "A fail case test. fail to listen\n" );
		return -1;
	}
	
	unblock = 0;	
	//设置套接字为阻塞
	if( sw_tcp_ioctl( lis_skt, FIONBIO, &unblock ) < 0 )
	{
		test_log( FAIL , "A fail case test. fail to  set block\n" );
		return -1;
	}
	OS_LOG_DEBUG( "set unblock\n" );
	
	//接收	
	if( ( ser_skt = sw_tcp_accept( lis_skt, &m_ip, &m_port ) ) < 0 )
	{	
		test_log( FAIL , "A fail case test. fail to  accept\n" );
		return -1;
	}
	OS_LOG_DEBUG( "accept\n" );

	//建立并打开test
	if( ( fd = open( "test", O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU ) ) < 0 )
	{
		test_log( FAIL , "A fail case test. fail to open test\n" );
		return -1;
	}
	OS_LOG_DEBUG( "open the test\n" );

	while(1)
	{
		//检查套接字是否可以接收
		if( sw_tcp_select( ser_skt, &recv_fds, NULL, NULL, 10 ) > 0 )
		{
			if( FD_ISSET( ser_skt, &recv_fds ) )
			{
				if( ( recv_size = sw_tcp_recv( ser_skt, recv_buf, sizeof(recv_buf) ) ) > 0 )
				{
					write( fd, recv_buf, recv_size ); 
				}	
				else
				{
					break;
				}
			}
		}
	}
	close( fd );
	sw_tcp_close( ser_skt );

	return 0;
}

/** 
 * @brief 测试客户端
 *   
 * @return 成功返回0;失败返回-1
 */   
static int test_tcp_client( void )
{	
	
	int cli_skt;
	char send_buf[1024];
	int fd;
	int send_size;
	unsigned long unblock;
	fd_set send_fds ;
	

	//创建套接字
	if( ( cli_skt = sw_tcp_socket() ) < 0 )
	{
		OS_LOG_ERROR( "client fail to socket\n" );
		return -1;
	}
	OS_LOG_DEBUG( "cli_skt=%d\n", cli_skt );

	//设置套接字为非阻塞
	if( sw_tcp_ioctl( cli_skt, FIONBIO, &unblock ) < 0 )
	{
		OS_LOG_ERROR( "client fail to  set unblock\n" );
		return -1;
	}
	OS_LOG_DEBUG( "set unblock\n" );

	//等待服务器
	usleep(1000);

	//连接服务器
	if( sw_tcp_connect( cli_skt, m_ip, m_port ) < 0 )
	{	
		OS_LOG_ERROR( "client fail to  connect\n" );
		return -1;
	}
	OS_LOG_DEBUG( "connect\n" );
	
	//打开文件file
	if( ( fd = open( m_file , O_RDONLY ) ) < 0 )
	{
		OS_LOG_ERROR( "client fail to open file\n" );
		return -1;
	}
	OS_LOG_DEBUG( "open the file  \n" );

	while(1)
	{
		//检查套接字是否可以发送
		if( sw_tcp_select( cli_skt, NULL, &send_fds, NULL, 10 ) > 0 )
		{
			if( FD_ISSET( cli_skt, &send_fds ) )
			{
				if( ( send_size = read( fd, send_buf, sizeof(send_buf) ) ) > 0 )
				{
					sw_tcp_send( cli_skt, send_buf, send_size );
				}
				else
				{
					break;
				}
			}
		}
	}

	close(fd);
	sw_tcp_close( cli_skt );

	return 0;
}

/** 
 * @brief 测试tcp
 *   
 * @return 成功返回PASS;失败返回FAIL
 */  
static int test_tcp( void )
{
	int pid;
	int status;
	int res;

	if( ( pid = fork() ) < 0 )
	{
		test_log( FAIL , "fail to fork\n" );
		return FAIL;
	}
	else if( pid > 0 )
	{
		res = test_tcp_server();
		wait( &status );                //获取子进程退出状态
	}
	else
	{
		if( test_tcp_client() < 0 )     //判断test_tcp_client是否运行成功
		{
			exit(-1);
		}
		else
		{   
			exit(0);
		}
	}

	if( res || status )                 //判断测试用例是否成功
	{
		return FAIL;
	}
	else
	{
		return PASS;
	}

}

/** 
 * @brief init
 *   
 */   
int test_init( void )
{
	//strcpy( g_test_info.m_name, "swtcp" );

	return 0;
}

/** 
 * @brief exit
 *   
 */   
void test_exit( void )
{
	return;
}

/** 
 * @brief 测试程序开始
 *
 * @param argc 传递参数个数 
 * @param *argv[] 参数数组
 *
 * @return 成功返回0，失败返回-1
 */   
int test_main( int argc, char *argv[] )
{
	sw_log_init( LOG_LEVEL_ALL, "console", "all" );
	if( argc < 4 )
	{
		OS_LOG_ERROR("less arg ...usage: ./file ip port filename\n" );
		return 0;
	}

	m_ip = inet_addr( argv[1] );
	m_port = htons( atoi( argv[2]) );

	strcpy( m_file, argv[3]);
	
	do_case( test_tcp, "test tcp" ); 

	return 0;
}
