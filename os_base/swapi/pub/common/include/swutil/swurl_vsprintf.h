/** 
 * @file swurl_vsprintf.h
 * @brief 定义生成url串的snprintf函数
 * @author sunniwell
 * @date 2014-10-06
 */

#ifndef __SWURL_VSPRINTF_H__
#define __SWURL_VSPRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
 @brief:用于生成url，与普通vsnprintf的区别是，这会将url中的非打印字符，和 '%'、'?'、'&'、'@'、':'、'/'、';' 这七个特殊字符转换成 %XX 的形式。其中XX为十六进制数，也就是该特殊字符的ASCII码
 */
int sw_url_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/*
 @brief:用于生成url，与普通snprintf的区别是，这会将url中的非打印字符，和 '%'、'?'、'&'、'@'、':'、'/'、';' 这七个特殊字符转换成 %XX 的形式。其中XX为十六进制数，也就是该特殊字符的ASCII码
 */
int sw_url_snprintf(char * buf, size_t size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SWURL_VSPRINTF_H__ */

