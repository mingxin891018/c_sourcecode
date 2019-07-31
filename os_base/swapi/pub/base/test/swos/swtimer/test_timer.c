
/**                                                                                                                          
  * @file test_timer.c
  * @brief test timer.c
  * @author xusheng
  * @date 2010-07-05
  * @history
  *      2010-07-05 created
  */

#include "swthrd.h"
#include "swlog.h"
#include "swos.h"
#include "swsignal.h"
#include "skeleton.h"
#include "swtimer.h"

static void timer_handle1( uint32_t wparam, uint32_t lparam )
{
//	OS_LOG_INFO("timer1 is counting %d,now time is %d !!\n", count1++,sw_thrd_get_tick()  );
	OS_LOG_INFO( "timer1 is counting! \n" );
}

static void timer_handle2( uint32_t wparam, uint32_t lparam )
{
	OS_LOG_INFO("timer2 is counting! \n" );
}

static void timer_handle3( uint32_t wparam, uint32_t lparam )
{
	OS_LOG_INFO("timer3 is counting! \n" );
}

static void timer_handle4(uint32_t wparam, uint32_t lparam )
{
	OS_LOG_INFO("timer4 is counting\n"  );
}

static void timer_handle5(uint32_t wparam, uint32_t lparam )
{
	OS_LOG_INFO("timer5 is counting\n" );
}

static void timer_handle6( uint32_t wparam, uint32_t lparam )
{
	OS_LOG_INFO("timer6 is counting \n" );
	OS_LOG_INFO("the frist param is %u,the second param is %u \n", wparam,lparam );
}

static void timer_handle7( uint32_t wparam, uint32_t lparam )
{
	OS_LOG_INFO("timer7 is counting!\n" );
}

//static void timer_handle8( uint32_t wparam, uint32_t lparam )
//{
//	OS_LOG_INFO("timer8 is counting!\n" );
//}

/** 
  * @brief test timer_create
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_create( )
{
	if( NULL == sw_timer_create( "swhandle1", 3000, 6, timer_handle1, 0, 0 ) )
	{
		OS_LOG_ERROR( "timer create failed\n" );

		return FAIL;
	}
	sleep(1);

	if( NULL == sw_timer_create( "swhandle2", 4000, 6, timer_handle2, 0, 0 ) )
	{
		OS_LOG_ERROR( "timer create failed\n" );

		return FAIL;
	}
	sleep(2);

	if( NULL == sw_timer_create( "swhandle3", 1500, 3, timer_handle3, 0, 0 ) )
	{
		OS_LOG_ERROR( "timer create failed\n" );

		return FAIL;
	}
	sleep(2);

	if( NULL == sw_timer_create( "swhandle4", 750, 5, timer_handle4, 0, 0 ) )
	{
		OS_LOG_ERROR( "timer create failed\n" );

		return FAIL;
	}
	
//	if( NULL == sw_timer_create( "swhandle8", 1000, -1, timer_handle8, 0, 0 ) )
//	{
//		OS_LOG_INFO( "timer create failed\n" );

//		return FAIL;
//	}
//	sleep(30);

    return PASS;
}


/** 
  * @brief test timer_create_exception
  *   
  * @return 成功返回PASS;
  *
  */  
int test_timer_create_exception()
{
	if( NULL != sw_timer_create( "swhandle1", -1, 5, timer_handle1, 0, 0 ) )
	{
		return FAIL;
	}
	if( NULL !=sw_timer_create( "swhandle2", 1000,-2 , timer_handle1, 0, 0 ) )
	{
		return FAIL;
	}

	if( NULL !=sw_timer_create( "swhandle3", 1000,1 , NULL , 0, 0 ) )
	{
		return FAIL;
	}
    
//	sw_timer_exit();
		
	return PASS;	
}



/** 
  * @brief test timer_init
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_init()
{
	int value;

	value = sw_timer_init(0,4096);	
	if( value == 0 )
	{	
		OS_LOG_INFO("testtimer   :  timer is inited \n");

		return PASS;
	}
	else
	{	
		OS_LOG_ERROR("testtimer   :   timer init failed \n");

		return FAIL;
	}
}


/** 
  * @brief test timer_init
  *   
  * @return 成功返回PASS;
  */  
int test_timer_init_rage()
{
	int value;

	value = sw_timer_init(255,64*4096);  
	if( value == 0 ) 
	{   
		OS_LOG_INFO("testtimer   :  timer is inited \n");

		return PASS;
	}   
	else
	{   
		OS_LOG_ERROR("testtimer   :   timer init failed \n");

		return FAIL;
	}   
}

/** 
  * @brief test timer_getbyname
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_get_byname(  )
{
	if( NULL == sw_timer_get_byname( "swhandle1" ) )
	{
		OS_LOG_INFO("dont find swhandle1's handle1\n");
	}
	else
	{
		OS_LOG_INFO("the handle1 is %p \n ", sw_timer_get_byname( "swhandle1" ));    
	}

	if( NULL == sw_timer_get_byname( "swhandle2" ) )
	{
		OS_LOG_INFO("dont find swhandle1's handle2\n");
	}
	else
	{
		OS_LOG_INFO("the handle2 is %p \n ", sw_timer_get_byname( "swhandle2" ));    
	}

	if( NULL == sw_timer_get_byname( "swhandle3" ) )
	{
		OS_LOG_INFO("dont find swhandle1's handle3\n");
	}
	else
	{
		OS_LOG_INFO("the handle3 is %p \n ", sw_timer_get_byname( "swhandle3" ));    
	}

	if( NULL == sw_timer_get_byname( "swhandle7" ) )
	{
		OS_LOG_INFO("dont find swhandle1's handle7\n");
	}
	else
	{
		OS_LOG_INFO("the handle4 is %p \n ", sw_timer_get_byname( "swhandle7" ));    
	}

	return PASS;

}

/** 
  * @brief test timer_getbyname_excetption
  *   
  * @return 成功返回PASS;
  */  
int test_timer_get_byname_exception(  )
{

    if( NULL == sw_timer_get_byname( NULL ) )
	{
		OS_LOG_ERROR("invalid name \n");

		return PASS;
	}
	else
	{
//		OS_LOG_INFO("the handle1 is %p \n ", sw_timer_get_byname( NULL ));

		return FAIL;	
	}

}

/** 
  * @brief test timer_set_status
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_set_status( )
{
	HANDLE tmp = sw_timer_create( "swhandle1", 2000, 6, timer_handle1, 0, 0 );

	if( -1 == sw_timer_set_status(tmp,  TIMER_STATUS_DISABLE ) )
	{
		OS_LOG_ERROR( "set status of swhandle1 is failed \n" );

		return FAIL;
	}
	else
	{
		OS_LOG_INFO( "set status of swhandle1 is sucess \n" );			
	}

	if( TIMER_STATUS_ENABLE  == sw_timer_get_status(tmp) )
	{
		OS_LOG_ERROR( "get status of swhandle1 is TIMER_STATUS_ENABLE  \n" );
		
		return FAIL;
	}
	else
	{
		OS_LOG_INFO("get status of swhandle1 is  TIMER_STATUS_DISABLE  \n");			
	}

	sleep(3);
     
	if( -1 == sw_timer_set_status(tmp,  TIMER_STATUS_ENABLE ) )
	{
		OS_LOG_ERROR( "set status of swhandle1 is failed\n" );

		return FAIL;
	}
	else
	{
		OS_LOG_INFO("set status of swhandle1 is sucess\n");			
	}

	if( TIMER_STATUS_ENABLE  == sw_timer_get_status(tmp) )
	{
		OS_LOG_INFO( "get status of swhandle1 is TIMER_STATUS_ENABLE  \n" );
	}
	else
	{
		OS_LOG_INFO("get status of swhandle1 is  TIMER_STATUS_DISABLE  \n");			
	}

	sleep(20);
	return PASS;
}


int test_timer_set_status_exception( )
{
	HANDLE tmp = sw_timer_create( "swhandle2", 2000, 6, timer_handle2, 0, 0 );

	if( -1 == sw_timer_set_status(tmp,  2 ) ) 
	{   
		OS_LOG_ERROR( "set status of swhandle1 is failed \n" );
	
	}   
	else
	{   
		OS_LOG_INFO( "set status of swhandle1 is sucess \n" ); 
		
		return FAIL;		
	}
   	
	sleep(13);

  	return PASS;	
}

/** 
  * @brief test timer set reset
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_reset( )
{
	HANDLE tmp;

	tmp = sw_timer_create( "swhandle3", 1500, 6, timer_handle3, 0, 0 );
	sleep(4);
	if( 0 == sw_timer_reset( tmp ) )
	{
		sleep(20);
		return PASS;
	}
	else
	{
		return FAIL;
	}
}	


/** 
  * @brief test timer set interval
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_set_interval( )
{
	HANDLE tmp;

	tmp = sw_timer_create( "swhandle4", 750, 4, timer_handle4, 0, 0 );
	sleep(2);
	if( 0 == sw_timer_set_interval(tmp,1500 ,6) )
	{
		sleep(20);

		return PASS;
	}
	else
	{
		return FAIL;
	}

}	

/** 
  * @brief test timer set interval
  *   
  * @return 成功返回PASS;
  */  
int test_timer_set_interval_exception( )
{
	HANDLE tmp;

	tmp = sw_timer_create( "swhandle4", 750, 4, timer_handle4, 0, 0 );
	sleep(2);

	if( sw_timer_set_interval(tmp,-1500 ,-6) == -1) 
	{
		return PASS;	
	}
	else
	{
		return FAIL;
	}

}	

/** 
  * @brief test timer destroy
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_destroy()
{
	HANDLE tmp;

	tmp = sw_timer_create( "swhandle7", 750, 4, timer_handle7, 0, 0 );
	sleep(2);
	sw_timer_destroy(tmp);
	OS_LOG_INFO( "swtimer is destroyed \n" );

	sleep(10);

	return PASS;
}


/** 
  * @brief test timer exit
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_exit()
{
	sw_timer_exit();
	OS_LOG_INFO("swtimer is exit \n");	

	return PASS;
}


/** 
  * @brief test timer set ontime  handler
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_set_ontimehandler( )
{
	HANDLE tmp;

	tmp = sw_timer_create( "swhandle5", 2000, 4, timer_handle5, 0, 0 );
    sleep(3);	
	if( -1 == sw_timer_set_ontimehandler( tmp,timer_handle6,2,5 ) )
	{
		return FAIL;
	}
	
	sleep(15);
	return PASS;
}

/** 
  * @brief test timer OS_LOG_INFO
  *   
  * @return 成功返回PASS;
  */  
int test_sw_timer_print()
{
	OS_LOG_INFO( "display timer info\n" );
	sw_timer_create( "swhandle5", 2000, 4, timer_handle5, 0, 0 );
	sw_timer_create( "swhandle6", 3000, 4, timer_handle7, 0, 0 );
	sw_timer_print();

	sleep(15);
	return PASS; 
}


/** 
  * @brief test init
  * 
  */
int test_init( void )
{
	//strcpy( g_test_info.m_name, "swtimer" );

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
  * @brief test swtimer 
  * 
  * @return 成功返回PASS
  */
int test_main( int argc, char *argv[] )
{
	sw_log_init( LOG_LEVEL_ALL, "console", "all" );
	do_case( test_timer_init_rage, "test_timer_init_rage function" );
	do_case( test_sw_timer_exit , " test_sw_timer_exit  function" );
	sleep(1);

	do_case( test_sw_timer_init , "test_sw_timer_init function" );

	do_case( test_timer_create_exception, "test_sw_timer_init_exception function" );

	do_case( test_sw_timer_create , " test_sw_timer_create  function" );
	do_case( test_sw_timer_get_byname , " test_sw_timer_get_byname function" );
	do_case( test_timer_get_byname_exception , " test_timer_get_byname_exception function" );
	sleep(30);
	
	do_case( test_sw_timer_set_status , " test_sw_timer_set_status  function" );
	do_case( test_timer_set_status_exception , " test_timer_set_status_exception function" );

	do_case( test_sw_timer_reset , "test_sw_timer_reset  function" );

	do_case( test_sw_timer_set_interval , " test_sw_timer_set_interval  function" );

	do_case( test_timer_set_interval_exception , " test_sw_timer_set_interval exception  function" );
	sleep(5);

	do_case( test_sw_timer_set_ontimehandler , " test_sw_timer_set_ontimehandler function" );

	do_case( test_sw_timer_destroy , " test_sw_timer_destroy function" );

	do_case( test_sw_timer_print  , " test_sw_timer_print function" );

	do_case( test_sw_timer_exit , " test_sw_timer_exit  function" );

	return PASS;
}




