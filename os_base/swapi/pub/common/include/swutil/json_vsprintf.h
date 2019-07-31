/** 
 * @file json_vsprintf.h
 * @brief 定义形成json串的snprintf函数
 * @author sunniwell
 * @date 2014-10-06
 */

#ifndef __JSON_VSPRINTF_H__
#define __JSON_VSPRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
 @brief:用于形成json串，与普通vsnprintf的区别是，这会将json串中的单引号、双引号、反斜框、回车、换行这5种字符进行转换
 */
int json_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/*
 @brief:用于形成json串，与普通snprintf的区别是，这会将json串中的单引号、双引号、反斜框、回车、换行这5种字符进行转换
 */
int json_snprintf(char * buf, size_t size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __JSON_VSPRINTF_H__ */

