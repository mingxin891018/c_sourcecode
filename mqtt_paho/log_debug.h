/*================================================================
*   Copyright (C) 2019 All rights reserved.
*   File name: log_debug.h
*   Author: zhaomingxin@sunniwell.net
*   Creation date: 2019-01-10
*   Describe: null
*
*===============================================================*/
#ifndef __LOG_DEBUG_H__
#define __LOG_DEBUG_H__


#define LOG_DEBUG(format, args...) (printf (  "[I/%s %s %d] " format, __FILE__, __func__ , __LINE__, ##args))







#endif //__LOG_DEBUG_H__
