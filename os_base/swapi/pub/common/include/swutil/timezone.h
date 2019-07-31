/** 
 * @file util.h
 * @brief util库，使用swthrd/swmutex/swsem/swqueue的入口
 * @author tanzongren
 * @date 2006-01-05
 */

#ifndef __SWUTIL_H__
#define __SWUTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 将字符串形式的时区转换成二进制形式，单位分钟 8:30 -> 8*60+30*/
int sw_timezone2minute( char* p_timezone );
	
/* 将二进制时区(单位分钟)转换成字符串 */
void sw_minute2timezone( int minute, char* buf );
	
#ifdef __cplusplus
}
#endif

#endif /* __SWUTIL_H__ */
