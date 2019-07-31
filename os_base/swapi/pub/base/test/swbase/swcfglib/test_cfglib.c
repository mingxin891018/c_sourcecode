/**                                                                                                             
  * @file   test_cfglib.c                                                                                           
  * @brief  收集配置库信息                                                                                           
  * @author xusheng                                                                                                              
  * @history                                                                                        
  *          2010-06-30  modified                                                                                             
  */                                                                                                                                  
                                                                                                                                     
#include "swapi.h"                                                                                                                   
#include "swbase.h"
#include "swcfglib.h"     
#include "skeleton.h"


static int lib_count;

/** 
  * @brief test 
  *
  * @return 成功返回PASS
  */    
int  test_sw_cfglib_init( void )
{
	if( 0 != sw_cfglib_init() )
	{

		printf("sw_cfglib init failed\n");
		
		return FAIL;
	}
	else
	{
		sw_cfglib_print();

		printf("sw cfglib init sucess\n");
	}

	return PASS;
}


/** 
  * @brief test 
  *
  * @return 成功返回PASS
  */    
int test_sw_cfglib_exit( void )
{
	return PASS;
}


/** 
  * @brief test 
  *
  * @return 成功返回PASS
  */    
int   test_sw_cfglib_add_info( void )
{

	if( -1 ==  sw_cfglib_add_info( SW_CFG_LIB_INFO ))
	{	
		printf("sw_cfglib add info failed\n");
		return FAIL;
	}
	else
	{
		printf("sw cfglib add info sucess\n");
	}

	return PASS;
}

/** 
  * @brief test 
  *
  * @return 成功返回PASS
  */    
int  test_sw_cfglib_print( void )
{
   	sw_cfglib_print();

	return PASS;
}

/** 
  * @brief test 
  *
  * @return 成功返回PASS
  */    
int  test_sw_cfglib_get_count( void )
{

	lib_count = sw_cfglib_get_count();
	printf("LIB COUNT IS %d \n", lib_count );

	return PASS;
}

/** 
  * @brief test 
  *
  * @return 成功返回PASS
  */    
int   test_sw_cfglib_get_info( void )
{
	int index;

	for( index = 0 ; index < lib_count; index++ )
	{
		printf("swcflib info %d is %s \n",index, sw_cfglib_get_info( index ) );
	}

	return PASS;
}

/** 
  * @brief test init
  * 
  */ 

int test_init( void )
{
	strcpy( g_test_info.m_name, "swcfglib" );

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
	do_case( test_sw_cfglib_init , " sw_cfglib_init  function" ); 
	do_case( test_sw_cfglib_add_info , "sw_cfglib_add_info  function" ); 
	do_case( test_sw_cfglib_get_count , "sw_cfglib_get_count  function" ); 
	do_case( test_sw_cfglib_get_info , "sw_cfglib_get_info  function" ); 
	do_case( test_sw_cfglib_print , "sw_cfglib_print  function" ); 
	do_case( test_sw_cfglib_exit , "sw_cfglib_exit  function" ); 

	return 0;
}                    
