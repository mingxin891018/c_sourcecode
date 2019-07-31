/** 
 * @file txtparser.h
 * @brief 对文本格式进行分析
 * @author chenkai
 * @date 2005-10-18 created
 */

#ifndef __TXTPARSER_H__
#define __TXTPARSER_H__


/* 找到下一个不为' '的字符 */
#define ADV_SPACE(a) {while (isspace(*(a)) && (*(a) != '\0'))(a)++;}

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 分析字符串是不是ip地址,本函数不会判断是否时合法的IP地址类,如0.0.0.1会认为时合法的ip地址
 * 
 * @param buf 指向的字符串
 * 
 * @return 分析后若是ip地址,返回true; 否则,返回false
 */
 bool sw_txtparser_is_address( char* buf );

/** 
 * @brief 提取等式的左右两边的字符串
 * 
 * @param equations 指向的源字符串
 * @param pleft 指向的等式左边字符串
 * @param pright 指向的等式右边字符串
 * 
 * @return 
 */
char* equation_parse(char* equations,char** pleft,char** pright);

/** 
 * @brief 提取等式的左右两边的字符串
 * 
 * @param equations 指向的源字符串
 * @param pleft 指向的等式左边字符串
 * @param pright 指向的等式右边字符串
 * 
 * @return 
 */
char* equation_parse_as_line(char* equations,char** pleft,char** pright);

/** 
 * @brief 把数字字符串转化为2进制数组
 * 
 * @param string 指向的数字字符串
 * @param length 数字字符串的长度
 * @param binary 指向的2进制数组
 * @param binsize 2进制数组的长度
 * 
 * @return 成功,返回0; 否则,返回-1
 */
int  txt2hex(const char* string, int length,uint8_t* binary,int binsize);

/** 
 * @brief 是否是一个整数
 * 
 * @param srcint 指向一个数值
 * 
 * @return 若是整数,返回true; 否则,返回false
 */
bool sw_txtparser_is_int( char* srcint );

/** 
 * @brief 参数有效性检查是否是Mac
 * 
 * @param srcMac 指向的字符串
 * 
 * @return 若是Mac地址,返回true; 否则,返回false
 */
bool sw_txtparser_is_mac_address( char* src_mac );

/** 
 * @brief 是否是带宽
 * 
 * @param srcBand 指向的字符串
 * 
 * @return 若是带宽,返回true; 否则,返回false
 */
bool sw_txtparser_is_band( char* src_band );

/** 
 * @brief 参数有效性检查 是否是一个port
 * 
 * @param srcport 指向的字符串
 * 
 * @return 若是port,返回true; 否则,返回false
 */
bool sw_txtparser_is_port( char* src_port );

/** 
 * @brief 参数有效性检查 是否是音量值
 * 
 * @param srcVol 指向的字符串
 * 
 * @return 若是音量值,返回true; 否则,返回false
 */
 bool sw_txtparser_is_vol( char* src_val );

/** 
 * @brief 参数有效性检查 是否是netmode
 * 
 * @param srcNetmode 指向的字符串
 * 
 * @return 若是netmode,返回true; 否则,返回false
 */
bool sw_txtparser_is_net_mode( char* src_net_mode );

#ifdef __cplusplus
}
#endif


#endif //__TXTPARSER_H__
