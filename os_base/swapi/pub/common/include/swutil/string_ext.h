/** 
 * @file string_ext.h
 * @brief 定义string的扩展函数
 * @author sunniwell
 * @date 2006-01-15
 */

#ifndef __STRING_EXT_H__
#define __STRING_EXT_H__

#ifdef __cplusplus
extern "C"
{
#endif
/** 
 * @brief 不区分大小写比较字符串的前n个字符
 * 
 * @param s1 指向的第一个字符串
 * @param s2 指向的第二个字符串
 * @param n 参加比较的字符数
 * 
 * @return 若参数s1和s2字符串相同则返回0。s1若大于s2则返回大于0的值。s1若小于s2则返回小于0的值
 */
int xstrncasecmp(const char *s1, const char *s2, size_t n);

/** 
 * @brief 不区分大小比较字符串
 * 
 * @param s1 指向的第一个字符串
 * @param s2 指向的第二个字符串
 * 
 * @return 若参数s1和s2字符串相同则返回0。s1若大于s2则返回大于0的值。s1若小于s2则返回小于0 的值
 */
int xstrcasecmp(const char *s1, const char *s2);

/** 
 * @brief 把字符串按delim定义的字符分割，并返回分割后的第一个串
 * 
 * @param stringp 指向欲分割的字符串
 * @param delim 字符串分割符
 * 
 * @return 返回下一个分割后的字符串指针，如果已无从分割则返回NULL
 */
char* xstrsep(char** stringp, const char *delim);

/** 
 * @brief 把最多size个字符输入到字符串
 * 
 * @param str 指向的字符串数组
 * @param size 复制的字节个数
 * @param format 所指字符串来转换并格式化数据
 * @param ... 无限一定类型的参数
 * 
 * @return 成功则返回参数str字符串长度，失败则返回-1
 */
int xsnprintf(char *str, size_t size, const char *format, ...);
/** 
 * @brief 把最多size个字符输入到字符串
 * 
 * @param s 指向的字符串数组
 * @param buf_size 复制的字节个数
 * @param format 所指字符串来转换并格式化数据
 * @param ap 可变参数列表
 * 
 * @return 成功则返回参数s字符串长度，失败则返回-1
 */
int xvsnprintf(char *s, size_t buf_size, const char* format, va_list ap);


/** 
 * @brief 提取字段值
 * 
 * @param buf 指向的源字符串
 * @param name 所指的参数名
 * @param value 所指的参数值
 * @param valuelen 所指参数值的长度
 * 
 * @return 成功,返回指向的参数值; 否则,返回NULL
 */
char *xstrgetval( char *buf, char *name, char *value, int valuelen );

/*
 @brief:判断输入的字符串是否是数字
 */
int xstrisdigit(char* str);

#ifdef __cplusplus
}
#endif

#endif /* __STRING_EXT_H__ */

