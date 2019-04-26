#ifndef _DEBUGLOG_H
#define _DEBUGLOG_H
  
#define SW_LOG_INFO(format, args...) (printf (  "[I/%s %s %d] " format, __FILE__, __func__ , __LINE__, ##args))
   
#define SW_LOG_DEBUG(format, args...) (printf(  "[D/%s %s %d] " format, __FILE__, __func__ , __LINE__, ##args))
    
#define SW_LOG_WARN(format, args...) (printf (  "[W/%s %s %d] " format, __FILE__, __func__ , __LINE__, ##args))
	 
#define SW_LOG_ERROR(format, args...) (printf(  "[E/%s %s %d] " format, __FILE__, __func__, __LINE__, ##args))
	  
#endif
