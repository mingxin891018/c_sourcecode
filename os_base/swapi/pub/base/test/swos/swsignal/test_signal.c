/**
 * @file test_sync.c
 * @brief test sync
 * @author qushihui
 * @date 2010-07-06
 * @history
 *		2010-07-20 created
 */
#include "swthrd.h"
#include "swlog.h"
#include "swos.h"
#include "swsignal.h"
#include "skeleton.h"

static int m_count = 0; 			//被保护变量
static pthread_t m_tid;				//线程号
static HANDLE m_h;					//信号量

/**                                                                                                                                                                
 * @brief test signal 
 * 
 */ 
static void* thread_fun( )
{
	OS_LOG_DEBUG( "thread1 start \n" );

	OS_LOG_DEBUG( "thread1 reset sem m_count=%d \n", m_count );
	sw_signal_reset( m_h );
	
	usleep(1000);		//睡1ms，释放系统资源给线程2
	OS_LOG_DEBUG( "thread1 sleep 1 ms \n" );

	if( sw_signal_wait( m_h, 10) )
	{
		OS_LOG_DEBUG( "thread1 fail to get sem \n" );
	}
	else
	{
		OS_LOG_DEBUG( "thread1 get sem m_count=%d \n", m_count );
	}

	sleep(1);			//睡1s，释放资源，测试线程2获取信号量超时
	OS_LOG_DEBUG( "sleep 1 s \n" );

	m_count++;			//获取信号量后更改m_count
	OS_LOG_DEBUG( "thread1 change m_count=%d \n", m_count );

	sw_signal_give( m_h );
	OS_LOG_DEBUG( "thread1 post sem m_count=%d \n", m_count );

	return NULL;
}

/**                                                                                                                                                                
 * @brief test signal 
 * 
 * @return 成功返回PASS,失败返回FAIL
 */ 
static int test_signal( void )
{
	if( !( m_h = sw_signal_create( 0, 1 ) ) )
	{
		test_log( FAIL, "fail to create signal\n");
		return FAIL;
	}
	pthread_create( &m_tid, NULL, thread_fun, NULL);
	
	OS_LOG_DEBUG( "thread2 start\n" );
	if( sw_signal_wait( m_h, 10) )
	{
		OS_LOG_DEBUG( "thread2 fail to get sem \n" );
	}
	else
	{
		OS_LOG_DEBUG( "thread2 get sem m_count=%d \n", m_count );
	}

	m_count++;			//获取信号量，更改m_count
	OS_LOG_DEBUG( "thread2 change m_count=%d \n", m_count );
	
	usleep(1000);		//睡1ms，释放资源，观察线程1是否能获取信号量
	OS_LOG_DEBUG( "thread get sem sleep 1 ms \n" );		

	sw_signal_give( m_h );
	OS_LOG_DEBUG( "thread2 post sem m_count=%d \n", m_count );

	usleep(1000);
	OS_LOG_DEBUG( "thread2 post sem sleep 1 ms \n" );	//睡1ms，释放资源，观察线程1是否能获取信号量

	if( sw_signal_wait( m_h, 10) )
	{
		OS_LOG_DEBUG( "thread2 fail to get sem \n" );
	}
	else
	{
		OS_LOG_DEBUG( "thread2 get sem m_count=%d \n", m_count );
	}

	pthread_join( m_tid, NULL ); 
	OS_LOG_DEBUG( "thread1 over  m_count=%d \n", m_count );

	sw_signal_destroy( m_h );
	OS_LOG_DEBUG( "sem destroy  m_count=%d \n", m_count );

	return PASS;
}

/**                                                                                                                                                                
 * @brief test init 
 * 
 */ 
int test_init( void )
{
	//strcpy( g_test_info.m_name, "swsignal" );

	return 0;
}

/**                                                                                                                                                                
 * @brief test exit
 * 
 */ 
void test_exit( void )
{
	return;
}

/**                                                                                                                                                                
 * @brief test start
 * 
 */ 
int test_main( int argc, char *argv[] )
{
	sw_log_init( LOG_LEVEL_ALL, "console", "all" );
	do_case( test_signal, "test signal function" ); 

	return 0;
}
