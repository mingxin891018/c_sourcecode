#ifndef __SWUTIL_PRIV_H__
#define __SWUTIL_PRIV_H__

#include "swlog.h"

#if 0
#define UTIL_LOG_DEBUG( format, ...) 	sw_log( LOG_LEVEL_DEBUG, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_INFO( format, ... ) 	sw_log( LOG_LEVEL_INFO, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_WARN( format, ... ) 	sw_log( LOG_LEVEL_WARN, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_ERROR( format, ... ) 	sw_log( LOG_LEVEL_ERROR, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_FATAL( format, ... ) 	sw_log( LOG_LEVEL_FATAL, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#endif
#define UTIL_LOG_DEBUG( format, ...) 	sw_log_syslog( LOG_LEVEL_DEBUG, LOG_TYPE_RUN, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_INFO( format, ... ) 	sw_log_syslog( LOG_LEVEL_INFO, LOG_TYPE_RUN, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_WARN( format, ... ) 	sw_log_syslog( LOG_LEVEL_WARN, LOG_TYPE_RUN, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_ERROR( format, ... ) 	sw_log_syslog( LOG_LEVEL_ERROR, LOG_TYPE_RUN, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )
#define UTIL_LOG_FATAL( format, ... ) 	sw_log_syslog( LOG_LEVEL_FATAL, LOG_TYPE_RUN, "util", __FILE__, __LINE__, format, ##__VA_ARGS__  )

#endif //__SWUTIL_PRIV_H__


