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

/** 
 * @brief 将所有字符转化成小写
 * 
 * @param buf 指向的源字符串
 * 
 * @return 成功,返回小写字符串;否则,返回NULL
 */
char *strlower( char * buf );

/** 
 * @brief 将所有字符转化成大写
 * 
 * @param buf 指向的源字符串
 * 
 * @return 成功,返回大写字符串;否则,返回NULL
 */
char *strupper(char * buf);

/** 
 * @brief 获取指定格式的时间
 * 
 * @param fmt 指向的转换格式化数据
 * 
 * @return 成功,返回转换后时间;否则,返回NULL
 */
char *getfmtdatetime(char *fmt);
/*  */
/** 
 * @brief 去除字符串中的指定字符
 * 
 * @param str 指向的源字符串
 * @param trim 指向的分割符
 *
 * @return 成功,返回分割后的字符串;否则,返回NULL
 */
char *strtrim(char *str, const char *trim);
char *strrtrim(char *str, const char *trim);
char *strltrim(char *str, const char *trim);

/** 
 * @brief 字符串比较(不区分的大小写)
 * 
 * @param cs 指向的第一个字符串
 * @param ct 指向的第二个字符串
 * 
 * @return 若参数cs和ct字符串相同,返回0;若cs大于ct,则返回大于0的值;若cs小于ct,则返回小于0的值
 */
int stricmp(const char * cs,const char * ct);
char *get_last_path_component(char *path);
int sw_crc32(register int fd, uint32_t *main_val, unsigned long *main_len);


/* 将字符串形式的时区转换成二进制形式，单位分钟 8:30 -> 8*60+30*/
int TimezoneToMinute( char* szTimezone );
	
/* 将二进制时区(单位分钟)转换成字符串 */
void MinuteToTimezone( int minute, char* buf );
	
#ifdef __cplusplus
}
#endif

#endif /* __SWUTIL_H__ */
