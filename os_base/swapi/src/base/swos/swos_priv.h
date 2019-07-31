#ifndef __SWOS_PRIV_H__
#define __SWOS_PRIV_H__

#include "swlog.h"
#if 0
#define OS_LOG_DEBUG( format, ...) 	sw_log( LOG_LEVEL_DEBUG, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_INFO( format, ... ) 	sw_log( LOG_LEVEL_INFO, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_WARN( format, ... ) 	sw_log( LOG_LEVEL_WARN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_ERROR( format, ... ) sw_log( LOG_LEVEL_ERROR, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_FATAL( format, ... ) sw_log( LOG_LEVEL_FATAL, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#endif
#define OS_LOG_DEBUG( format, ...) 	sw_log_syslog( LOG_LEVEL_DEBUG, LOG_TYPE_RUN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_INFO( format, ... ) 	sw_log_syslog( LOG_LEVEL_INFO, LOG_TYPE_RUN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_WARN( format, ... ) 	sw_log_syslog( LOG_LEVEL_WARN, LOG_TYPE_RUN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_ERROR( format, ... ) sw_log_syslog( LOG_LEVEL_ERROR, LOG_TYPE_RUN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_FATAL( format, ... ) sw_log_syslog( LOG_LEVEL_FATAL, LOG_TYPE_RUN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )

#endif //__SWOS_PRIV_H__

