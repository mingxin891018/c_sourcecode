#ifndef __SWCOMMON_PRIV_H__
#define __SWCOMMON_PRIV_H__

#include "swlog.h"
#if 0
#define SWCOMMON_LOG_DEBUG(...) 	sw_log( LOG_LEVEL_DEBUG, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_INFO(... ) 	sw_log( LOG_LEVEL_INFO, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_WARN(... ) 	sw_log( LOG_LEVEL_WARN, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_ERROR(... ) 	sw_log( LOG_LEVEL_ERROR, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_FATAL(... ) 	sw_log( LOG_LEVEL_FATAL, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#endif
#define SWCOMMON_LOG_DEBUG(...) 	sw_log_syslog( LOG_LEVEL_DEBUG, LOG_TYPE_RUN, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_INFO(... ) 	sw_log_syslog( LOG_LEVEL_INFO, LOG_TYPE_RUN, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_WARN(... ) 	sw_log_syslog( LOG_LEVEL_WARN, LOG_TYPE_RUN, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_ERROR(... ) 	sw_log_syslog( LOG_LEVEL_ERROR, LOG_TYPE_RUN, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )
#define SWCOMMON_LOG_FATAL(... ) 	sw_log_syslog( LOG_LEVEL_FATAL, LOG_TYPE_RUN, "COMMON", __FUNCTION__, __LINE__, __VA_ARGS__  )

#endif //__SWBASE_PRIV_H__
