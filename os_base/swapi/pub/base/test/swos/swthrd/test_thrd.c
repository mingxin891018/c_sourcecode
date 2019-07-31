/**
  * @file test_log.c
  * @brief test swlog.c
  * @author xusheng 
  * @date 2010-07-09
  * @history
  *      2010-07-09  created
  */

#include "swthrd.h"
#include "swos.h" 
#include "swlog.h"
#include "skeleton.h"
#include "swbase.h"
#include "swapi.h"


static bool m_is_exit = false;
static HANDLE m_thrd = NULL;

/**
  * @brief test thrd proc
  *
  * @return 成功返回true
  */ 
static bool test_proc( uint32_t wparam, uint32_t lparam )
{
		static int count = 0;

		OS_LOG_INFO("count = %d\n",count++ );
		sw_thrd_delay(1000 );

		if( m_is_exit )
		{	
			m_thrd = NULL;
			OS_LOG_INFO("thread  exit \n");

			return false;
		}

		return true;
}

/**
  * @brief test thrd open
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_open(void)
{
	m_thrd = sw_thrd_open( "test1", 20, 0, 4096, test_proc, 0, 0);
	
	if( NULL ==  m_thrd )
	{
		OS_LOG_ERROR( "thread open is failed\n" );

		return FAIL;
	}
	else
	{
		OS_LOG_INFO( "thread open is suceess\n" );
	}

	if( m_thrd )
	{
	   	sw_thrd_resume( m_thrd );
	
		OS_LOG_INFO( "thread is opened and resumed\n" );
	}
	sleep(2);

    return PASS;	
}


/**
  * @brief test thrd open
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_open_rage(void)
{
    HANDLE thrd = NULL;
	thrd = sw_thrd_open( "test1", 255, 2, 4096, test_proc, 0, 0);
	
	if( NULL ==  thrd )
	{
		OS_LOG_ERROR( "thread open is failed\n" );

		return FAIL;
	}
	else
	{
		OS_LOG_INFO("thread open is suceess\n");
	}

	if( thrd )
	{
	   	sw_thrd_resume( thrd );
	
		OS_LOG_INFO("thread is opened and resumed\n");
	}
	
	sw_thrd_close( thrd,2000 );

    return PASS;	
}


/**
  * @brief test thrd close
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_close(void)
{
	OS_LOG_INFO("it is going to exit thread test\n");

	if( m_thrd ) 
	{ 
		sw_thrd_close( m_thrd,2000 );
		OS_LOG_INFO("thrd is successful closed\n");
	}

	return PASS;
}

/**
  * @brief test thrd is_openned
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_is_openned(void)
{
	if( sw_thrd_is_openned( m_thrd ) == true )
	{
		OS_LOG_INFO("the thrd is open\n");
	}
	else
	{
		OS_LOG_INFO("the thread is not open\n");	
	}
	
	if( false == sw_thrd_is_openned( NULL ) )
	{
		OS_LOG_INFO("the thrd open exception test  suceess\n ");
	}
	else
	{
		OS_LOG_ERROR("the thrd open exception test  failed\n ");

		return FAIL;
	}

	return PASS;
}

/**
  * @brief test thrd get tick
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_get_tick(void)
{
    unsigned int now_time;
	now_time = sw_thrd_get_tick();

	OS_LOG_INFO("the current time is %u \n",now_time);

	return PASS;
}

/**
  * @brief test thrd pause
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_pause(void)
{
	 sw_thrd_pause( m_thrd );

	 if( sw_thrd_is_paused( m_thrd ) )
	 {
	 		OS_LOG_INFO("the thread is pause \n");
	 }
	 else
	 {
		 OS_LOG_ERROR("the thread pause is failed");

		 return FAIL;
	 }

	 sleep(5);
	 sw_thrd_resume( m_thrd );
	 OS_LOG_INFO("the thread is resumed \n");

	 return PASS;
}

/**
  * @brief test thrd delay
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_delay(void)
{
	OS_LOG_INFO("it is going to delay for 3 seconds\n");
	sw_thrd_delay(3000);	
	OS_LOG_INFO("it has delayed for 3 seconds\n");

	return PASS;
}

/**
  * @brief test thrd by name
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_find_byname(void)
{
	HANDLE handle;
	handle = sw_thrd_find_byname( "test1" );
	if( NULL == handle )
	{
		OS_LOG_ERROR("sw thread find by name is failed\n");

		return FAIL;
	}
	else
	{
		OS_LOG_INFO("the thread handle is %p \n", handle);
	}

	return PASS;
}

/**
  * @brief test thrd print
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_print(void)
{
	sw_thrd_print();	

	return PASS;
}

/**
  * @brief test thrd priority
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_set_priority( void )
{
	int priority;

	sw_thrd_set_priority( m_thrd , 255  );
	priority =(int)(sw_thrd_get_priority( m_thrd ));
	OS_LOG_INFO( "get the priority is %d\n", priority  );

	
	sw_thrd_set_priority( m_thrd , 20  );
	priority =(int)(sw_thrd_get_priority( m_thrd ));
	OS_LOG_INFO( "get the priority is %d\n", priority  );

	return PASS;
}

/**
  * @brief test thrd policy
  *
  * @return 成功返回PASS
  */ 
int test_sw_thrd_set_global_policy( void )
{
	sw_thrd_set_global_policy( 0 );

	return PASS;
}

/** 
  * @brief test init
  * 
  */    
int test_init( void )
{
	//strcpy( g_test_info.m_name, "swthrd" );

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
  * @brief start
  *   
  */

int test_main( int argc, char *argv[] )
{
	sw_log_init( LOG_LEVEL_ALL, "console", "all" );
	do_case( test_sw_thrd_open_rage , "test_sw_thrd_open rage function" ); 
	do_case( test_sw_thrd_open , "test_sw_thrd_open function" ); 
	do_case( test_sw_thrd_is_openned , "test_sw_thrd_is_openned function" ); 
	do_case( test_sw_thrd_print , "test_sw_thrd_print function" ); 
	do_case( test_sw_thrd_get_tick , "test_sw_thrd_get_tick function" ); 
	do_case( test_sw_thrd_pause , "test_sw_thrd_pause function" ); 
	do_case( test_sw_thrd_delay , "test_sw_thrd_delay function" ); 
	do_case( test_sw_thrd_find_byname , "test_sw_thrd_find_byname function" ); 
	do_case( test_sw_thrd_set_priority , "test_sw_thrd_set_priority function" ); 
	do_case( test_sw_thrd_set_global_policy , "test_sw_thrd_set_global_policy function" ); 
	do_case( test_sw_thrd_print , "test_sw_thrd_print function" ); 
	
	do_case( test_sw_thrd_close , "test_swthrd_close function" ); 

	return 0;
} 














 



