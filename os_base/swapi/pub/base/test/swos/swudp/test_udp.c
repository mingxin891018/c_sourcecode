/**
 * @file test_udp.c
 * @brief test udp.c
 * @author qushihui
 * @date 2010-07-05
 * @history
 *		2010-07-16 created
 */
#include "swudp.h"
#include "swos.h"
#include "swlog.h"
#include "skeleton.h"

static unsigned long  m_multicast_ip;        	//多播组ip 
static unsigned short m_port;					//多播组端口
static char m_file[24];							//传输的文件名

/** 
 * @brief 测试服务器端
 *   
 * @return 成功返回0;失败返回-1
 */   
int test_udp_server( void )
{
	int ser_skt;
	char recv_buf[1024];
	int fd;
	int recv_size;
	unsigned long unblock = 1;
	fd_set recv_fds;

	//生成套接字
	if( ( ser_skt = sw_udp_socket() ) < 0 )
	{
		test_log( FAIL, "server fail to socket\n" );		
		return -1;
	}
	OS_LOG_DEBUG( "ser_skt=%d\n", ser_skt );

	//设置套接字为非阻塞
	if( sw_udp_ioctl( ser_skt, FIONBIO, &unblock ) < 0 ) 
	{
		test_log( FAIL, "server fail to set unblock\n" );		
		return -1;
	}
	OS_LOG_DEBUG("server set unblock\n");

	//绑定
	if( sw_udp_bind( ser_skt, m_multicast_ip, m_port ) < 0)
	{
		test_log( FAIL, "server fail to  bind\n" );
		return -1;
	}
	OS_LOG_DEBUG( "bind the server socket\n" );

	//加入多播组
	if( sw_udp_join( ser_skt, m_multicast_ip ) < 0)
	{
		test_log( FAIL, "server fail to join\n" );
		return -1;
	}
	OS_LOG_DEBUG( "join the multicast\n" );

	//创建并打开文件test
	if( ( fd = open( "test", O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU ) ) < 0 ) 
	{
		test_log( FAIL, "server fail to open file\n" );		
		return -1;
	}
	OS_LOG_DEBUG( "open the test\n" );

	while(1)
	{
		//检查是否有套接字可以接收数据
		if(  sw_udp_select( ser_skt, &recv_fds, NULL, NULL, 10 ) > 0 )
		{
			if( FD_ISSET( ser_skt, &recv_fds ) )
			{
				//从套接字接收数据
				if( ( recv_size = sw_udp_recv( ser_skt, NULL, NULL, recv_buf, sizeof(recv_buf) ) ) > 0 )
				{
					write( fd, recv_buf, recv_size ); 
				}	
				else
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}

	//退出组播组
	if( sw_udp_drop( ser_skt, m_multicast_ip ) < 0 )
	{
		test_log( FAIL, "server fail to drop\n" );
		return -1;
	}
	OS_LOG_DEBUG( "drop the multicast\n" );
	
	close( fd );
	sw_udp_close( ser_skt );

	return 0;
}

/** 
 * @brief test client
 *   
 * @return 成功返回0;失败返回-1
 */   
int test_udp_client( void )
{	
	
	int cli_skt;
	char send_buf[1024];
	int fd;
	int send_size;
	unsigned long unblock = 1;
	fd_set send_fds ;
	

	//创建套接字
	if( ( cli_skt = sw_udp_socket() ) < 0)
	{
		OS_LOG_ERROR( "client fail to socket\n" );		
		return -1;
	}
	OS_LOG_DEBUG( "cli_skt=%d\n", cli_skt );
	
	//设置套接字为非阻塞
	if( sw_udp_ioctl( cli_skt, FIONBIO, &unblock) < 0 ) 
	{
		OS_LOG_ERROR( "client fail to set unblock\n" );		
		return -1;
	}
	OS_LOG_DEBUG( "set unblock\n" );

	//等待服务器
	usleep(1000);
	
	//打开要传输的文件
	if( ( fd = open( m_file , O_RDONLY ) ) < 0 )
	{
		OS_LOG_ERROR( "client fail to open file\n" );			
		return -1;
	}
	OS_LOG_DEBUG( "open the file  \n" );

	while(1)
	{
		//检查套接字是否可以发送
		if( sw_udp_select( cli_skt, NULL, &send_fds, NULL, 10 ) > 0 )
		{
			if( FD_ISSET( cli_skt, &send_fds ) )
			{
				if( ( send_size = read( fd, send_buf, sizeof(send_buf) ) ) > 0 )
				{
					sw_udp_send( cli_skt, m_multicast_ip, m_port, send_buf, send_size );
				}
				else
				{
					break;
				}
			}
		}
	}
	close(fd);
	sw_udp_close( cli_skt );

	return 0;
}

static int test_udp( void )
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
		res = test_udp_server();
		wait( &status );				//获取子进程退出状态
	}
	else
	{
		if( test_udp_client() < 0 )		//判断test_udp_client是否运行成功
		{
			exit(-1);
		}
		else
		{	
			exit(0);
		}
	}

	if( res || status )					//判断测试用例是否成功
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
	//strcpy( g_test_info.m_name, "swudp" );

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
	if( argc != 4 )
	{
		printf( "less arg ...usage: ./file multicast_ip  port filename\n" );
		return 0;
	}

	m_multicast_ip = inet_addr( argv[1] );
	m_port = htons( atoi( argv[2] ) );
	strcpy( m_file, argv[3]);
	
	do_case( test_udp, "test udp" );
	return 0;
}
