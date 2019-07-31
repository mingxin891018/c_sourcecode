/**
 * @file test-skeleton.c
 * @brief 
 * @author lilei
 * @date 2010-07-02
 * @history
 *		2010-07-02 created
 *		2010-07-15 qushihui modified
 */

#include "swapi.h"
#include "skeleton.h"

/** 
 * @brief Save the logging info, print it for test is stop.
 * 
 * @param level int, the level of the logging info
 * @param format char*, format string.
 * @param ... 
 */
void test_log( int level, const char *format, ... )
{
	va_list args;
	char buf[1024] = {0};
	va_start( args, format );
	vsprintf( buf, format, args );
	va_end(args);

	return;
}

/** 
 * @brief Invoke a test case, a case include same test cases.
 * 
 * @param test_case fun point,
 * @param info char*, provide print info for test running.
 * 
 * @return int , test status.
 */
void do_case( int (*test_case)(void), const char* info )
{
	int ret = 0;
	
	if ( info != NULL ) printf ( "Running Test : %s ... \n", info );
	ret = (*test_case)();  
}

int main( int argc, char *argv[] )
{

	test_main( argc, argv );
	return;
}
