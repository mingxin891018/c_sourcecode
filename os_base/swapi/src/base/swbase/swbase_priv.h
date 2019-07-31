#ifndef __SWBASE_PRIV_H__
#define __SWBASE_PRIV_H__

#include "swlog.h"
#if 0
#define BASE_LOG_DEBUG( format, ...) 	sw_log( LOG_LEVEL_DEBUG, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_INFO( format, ... ) 	sw_log( LOG_LEVEL_INFO, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_WARN( format, ... ) 	sw_log( LOG_LEVEL_WARN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_ERROR( format, ... ) 	sw_log( LOG_LEVEL_ERROR, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_FATAL( format, ... ) 	sw_log( LOG_LEVEL_FATAL, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#endif
#define BASE_LOG_DEBUG( format, ...) 	sw_log_syslog( LOG_LEVEL_DEBUG, LOG_TYPE_RUN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_INFO( format, ... ) 	sw_log_syslog( LOG_LEVEL_INFO, LOG_TYPE_RUN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_WARN( format, ... ) 	sw_log_syslog( LOG_LEVEL_WARN, LOG_TYPE_RUN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_ERROR( format, ... ) 	sw_log_syslog( LOG_LEVEL_ERROR, LOG_TYPE_RUN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define BASE_LOG_FATAL( format, ... ) 	sw_log_syslog( LOG_LEVEL_FATAL, LOG_TYPE_RUN, "base", __FILE__, __LINE__, format, ##__VA_ARGS__  )

#endif //__SWBASE_PRIV_H__


