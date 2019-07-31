/**
 * @file test_log.c
 * @brief test swlog.c
 * @author qushihui
 * @date 2010-07-09
 * @history
 *		2010-07-06 created
 */
#include "swapi.h"
#include "swlog.h"
#include "skeleton.h"
#include "swbase.h"

#define BASE_LOG_DEBUG( format, ...) 	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_INFO( format, ... ) 	sw_log( LOG_LEVEL_INFO, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_WARN( format, ... ) 	sw_log( LOG_LEVEL_WARN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_ERROR( format, ... ) 	sw_log( LOG_LEVEL_ERROR, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_FATAL( format, ... ) 	sw_log( LOG_LEVEL_FATAL, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
/** 
 * @brief test level
 *
 * @return 成功返回PASS，失败返回FAIL
 */    
static int test_level( void )
{
	
	if( sw_log_init( LOG_LEVEL_ALL, "console", "all" ) < 0 )
	{
		test_log( FAIL, "fail to init\n");
		return FAIL;
	}
	sw_log( LOG_LEVEL_ALL, "base", __FILE__, __LINE__, "level init:%d\n", sw_log_get_level() );
	sw_log( LOG_LEVEL_FATAL, "base", __FILE__, __LINE__, "level init:%d\n", sw_log_get_level() );
	
	sw_log_set_level( LOG_LEVEL_WARN );
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__, "set level:%d\n", sw_log_get_level() );
	sw_log( LOG_LEVEL_WARN, "base", __FILE__, __LINE__, "set level:%d\n", sw_log_get_level() );
	sw_log( LOG_LEVEL_FATAL, "base", __FILE__, __LINE__, "set level:%d\n", sw_log_get_level() );
	sw_log( LOG_LEVEL_FATAL, "os", __FILE__, __LINE__," set level:%d\n", sw_log_get_level() );
	
	sw_log_exit();

	return PASS;
}

/** 
 * @brief test target
 *
 * @return 成功返回PASS,失败返回FAIL
 */    
static int test_target( void )
{
	if( sw_log_init( LOG_LEVEL_ALL, "console", "all") < 0 )
	{
		test_log( FAIL, "fail to init\n");
		return FAIL;
	}
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"target init:%s\n", sw_log_get_targets() );
	
	sw_log_add_target("file:///tmp/log.log");
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"add target:%s\n", sw_log_get_targets() );

	sw_log_del_target("file:///tmp/log.log");
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"del target:%s\n", sw_log_get_targets() );
	
	sw_log_set_targets("console,file:///tmp/log.log,file:///tmp/another.log");
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"set target:%s\n", sw_log_get_targets() );
	
	sw_log_clear_alltarget();
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"clear alltarget:%s\n", sw_log_get_targets() );
	
	sw_log_exit();

	return PASS;
}

/** 
 * @brief test mods
 *
 * @return 成功返回PASS，失败返回FAIL
 */    
static int test_mods( void )
{
	if( sw_log_init( LOG_LEVEL_ALL, "console", "all") < 0 )
	{
		test_log( FAIL, "fail to init\n");
		return FAIL;
	}
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"init mods:%s\n", sw_log_get_mods() );
	sw_log( LOG_LEVEL_DEBUG, "media", __FILE__, __LINE__,"init mods:%s\n", sw_log_get_mods() );
	sw_log( LOG_LEVEL_DEBUG, "os", __FILE__, __LINE__,"init mods:%s\n", sw_log_get_mods() );
	
	sw_log_add_mod("media");
	sw_log( LOG_LEVEL_DEBUG, "media", __FILE__, __LINE__,"add mods:%s\n", sw_log_get_mods() );

	sw_log_del_mod("media");
	sw_log( LOG_LEVEL_DEBUG, "media", __FILE__, __LINE__,"del mods:%s\n", sw_log_get_mods() );
	
	sw_log_set_mods("base,os");
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"set mods:%s\n", sw_log_get_mods() );
	
	sw_log_clear_allmods();
	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"clear allmods:%s\n", sw_log_get_mods() );
	
	sw_log_add_allmods();
	sw_log( LOG_LEVEL_DEBUG, "swvedio", __FILE__, __LINE__,"add allmods:%s\n", sw_log_get_mods() );

	sw_log_exit();
	
	return PASS;
}

/** 
 * @brief test multi thread
 *
 */
static void* thread_fun_1( )
{
	int i;
	
	for( i = 0; i < 3; i++)
	{
		sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"thread_fun_1 %d times\n",i+1 );
	}

	return NULL;
}

/** 
 * @brief test multi thread
 *
 */
static void* thread_fun_2( )
{
	int i;
	
	for( i = 0; i < 3; i++)
	{
		sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"thread_fun_2 %d times\n",i+1 );
	}

	return NULL;
}

/** 
 * @brief test multi thread
 *
 */
static void* thread_fun_3( )
{
	int i;
	
	for( i = 0; i < 3; i++)
	{
		sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"thread_fun_3 %d times\n",i+1 );
	}

	return NULL;
}

/** 
 * @brief test multi thread
 *
 */
static void* thread_fun_4( )
{
	int i;
	
	for( i = 0; i < 3; i++)
	{
		sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"thread_fun_4 %d times\n",i+1 );
	}

	return NULL;
}

/** 
 * @brief test multi thread
 *
 */
static void* thread_fun_5( )
{
	int i;
	
	for( i = 0; i < 3; i++)
	{
		sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"thread_fun_5 %d times\n",i+1 );
	}

	return NULL;
}

/** 
 * @brief test multi thread
 *
 * @return 成功返回PASS,失败返回FAIL
 */
static int test_thread( void )
{
	pthread_t tid_1, tid_2, tid_3, tid_4, tid_5;
	
	if( sw_log_init( LOG_LEVEL_DEBUG, "console,file:///tmp/thread.log", "base" ) < 0 )
	{
		test_log( FAIL, "fail to init\n");
		return FAIL;
	}
	
	pthread_create( &tid_1, NULL, thread_fun_1, NULL);
	pthread_create( &tid_2, NULL, thread_fun_2, NULL);
	pthread_create( &tid_3, NULL, thread_fun_3, NULL);
	pthread_create( &tid_4, NULL, thread_fun_4, NULL);
	pthread_create( &tid_5, NULL, thread_fun_5, NULL);

	pthread_join( tid_5, NULL ); 

	return PASS;
}

/** 
 * @brief test filesize
 *
 * @return 成功返回PASS，失败返回FAIL
 */    
static int test_size( void )
{
	int i;

	//测试有日志大小规定的打印日志
	printf("======[%d]\n",__LINE__);
	if( sw_log_init( LOG_LEVEL_DEBUG, "file:///tmp/size.log&max_size=200k", "base" ) < 0 )
	{
		test_log( FAIL, "fail to init\n");
		return FAIL;
	}
	printf("======[%d]\n",__LINE__);
	sw_log_print_state();
	for( i = 0; i < 20 ; i++ )
	{
		sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__,"i= %d\n", i );
	}
	printf("======[%d]\n",__LINE__);
	sw_log_exit();

	return PASS;
}

/** 
 * @brief test macro
 *
 * @return 成功返回PASS
 */    
static int test_macro( void )
{
	int i = 0;
	if( sw_log_init( LOG_LEVEL_ALL, "console", "all") < 0 )
	{
		test_log( FAIL, "fail to init\n");
		return FAIL;
	}
	
	BASE_LOG_DEBUG("test macro\n" );
	BASE_LOG_INFO("test macro\n" );
	BASE_LOG_WARN("test macro\n" );
	BASE_LOG_ERROR("test macro\n" );
	BASE_LOG_FATAL("test macro\n" );
	BASE_LOG_DEBUG("test macro %s %d\n", "ok", i );

	return PASS;
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
	do_case( test_level, "test level function" ); 
	do_case( test_mods, "test mods function" ); 
	do_case( test_target, "test target function" ); 
	do_case( test_macro, "test macro function" ); 
	//do_case( test_thread, "test multi thread");
	do_case( test_size, "test size function" ); 

	return 0;
}

