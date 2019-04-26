#ifndef _DEBUGLOG_H
#define _DEBUGLOG_H
  
#define Msg_Info(format, args...) (printf (  "[%s-%s I/%s %s %d] "   format , __DATE__, __TIME__, __FILE__, __func__ , __LINE__, ##args))
   
#define Msg_Debug(format, args...) (printf(  "[%s-%s D/%s %s %d] "   format , __DATE__, __TIME__, __FILE__, __func__ , __LINE__, ##args))
    
#define Msg_Warn(format, args...) (printf (  "[%s-%s W/%s %s %d] "   format , __DATE__, __TIME__, __FILE__, __func__ , __LINE__, ##args))
	 
#define Msg_Error(format, args...) (printf(  "[%s-%s E/%s %s %d] "   format , __DATE__, __TIME__, __FILE__, __func__, __LINE__, ##args))
	  
#endif
