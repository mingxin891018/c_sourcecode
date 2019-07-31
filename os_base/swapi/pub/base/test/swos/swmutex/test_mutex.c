/**
 * @file test_sync.c
 * @brief test sync
 * @author qushihui
 * @date 2010-07-06
 * @history
 *		2010-07-21 created
 */
#include "swapi.h"
#include "swmutex.h"
#include "swlog.h"
#include "swos.h"
#include "swthrd.h"
#include "skeleton.h"

static int m_count = 0;   		//被保护变量
static pthread_t m_tid;			//线程号
static pthread_mutex_t* m_h;	//互斥锁


/** 
 * @brief test mutex
 *   
 */    
static void* thread_fun( )
{
	OS_LOG_DEBUG( "thread1 start \n" );
	
	m_count++;		//未加锁时m_count加1
	OS_LOG_DEBUG( "thread1 have no lock:m_count=%d \n", m_count );

	if ( m_h )
	{
		sw_mutex_lock( m_h );
		OS_LOG_DEBUG( "thread1 lock \n" );
	}
	m_count++;		//加锁后m_count加1
	OS_LOG_DEBUG( "thread1 lock m_count=%d \n", m_count );

	sleep(1);		//睡1s，释放系统资源，观察线程2是否能更改m_count

	if( m_h )
	{
		sw_mutex_unlock( m_h );
		OS_LOG_DEBUG( "thread1 unlock \n" );
	}

	return NULL;
}

/** 
 * @brief test mutex
 *   
 * @return 成功返回PASS,失败返回FAIL
 */    

int test_mutex( void )
{
	//创建互斥锁
	if( !( m_h = sw_mutex_create() ) )
	{
		test_log( FAIL, "a fail test.fail to create mutex\n");
		return FAIL;
	}
	//创建线程
	pthread_create( &m_tid, NULL, thread_fun, NULL );

	OS_LOG_DEBUG( "thread2 start \n" );

	if ( m_h )
	{
		sw_mutex_lock( m_h );
		OS_LOG_DEBUG( "thread2 lock \n" );
	}

	m_count++;			//加锁后m_count加1
	OS_LOG_DEBUG( "thread2 lock m_count=%d \n", m_count );
	
	sleep(1);			//睡1s，观察线程1是否能改变m_count

	if( m_h )
	{
		sw_mutex_unlock( m_h );
		OS_LOG_DEBUG( "thread2 unlock \n" );
	}
	pthread_join( m_tid, NULL ); 

	return PASS;
}

/** 
 * @brief test init
 * 
 */ 
int test_init( void )
{
	//strcpy( g_test_info.m_name, "swmutex" );

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
	do_case( test_mutex, "test mutex function" ); 

	return 0;
}
