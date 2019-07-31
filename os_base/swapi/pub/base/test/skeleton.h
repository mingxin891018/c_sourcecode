#ifndef SKELETON_H_
#define SKELETON_H_
#define PASS 0
#define FAIL -1

#define OS_LOG_DEBUG( format, ...) 	sw_log( LOG_LEVEL_DEBUG, "os", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_INFO( format, ... ) 	sw_log( LOG_LEVEL_INFO, "os", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_WARN( format, ... ) 	sw_log( LOG_LEVEL_WARN, "os", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_ERROR( format, ... ) 	sw_log( LOG_LEVEL_ERROR, "os", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_FATAL( format, ... ) 	sw_log( LOG_LEVEL_FATAL, "os", __FILE__, __LINE__, format, ##__VA_ARGS__  )
void test_log( int level, const char *format, ... );
void do_case( int (*test_case)(void), const char* info );
#endif
