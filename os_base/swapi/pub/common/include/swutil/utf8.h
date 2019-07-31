#ifndef __UTF8_H__
#define __UTF8_H__

/**
 * @brief 获取UTF8的单个字节所占长度
 * @return 是utf8返回长度，否则0
 */
int UTF8_CharLen(unsigned char* utf8_text);
/**
 * @brief 拷贝单个UTF8数据,并返回拷贝的内存长度
 *
 * @param utf8_dest 目标地址
 *		utf8_src源utf8子串起始地址
 *
 * @return 返回实际拷贝的utf8_src的个数.失败返回0
 */
int UTF8_charcpy(char *utf8_dest, char* utf8_src);

/** 
 * @brief UTF-8转Unicode，结果在unicode_text中
 * 
 * @param utf8_text 指向的源utf8字符串
 * @param unicode_text 指向转换后的unicode字符串
 * 
 * @return 返回实际使用的utf8_text的个数.如果返回0,要注意排除错误
 */
int UTF8_Unicode( unsigned char* utf8_text, unsigned short* unicode_text );
/** 
 * @brief 求utf8字符串的长度
 * 
 * @param utf8_text 指向的源utf8字符串
 * 
 * @return 返回utf8字符串的长度
 */
int UTF8_strlen(  unsigned char* utf8_text );
/** 
 * @brief 在utf8字符串中查找一个unicode码
 * 
 * @param utf8_text 指向的源utf8字符串
 * @param unicode 查找的unicode码
 * 
 * @return 查找成功,返回unicode码出现的位置;否则,返回NULL
 */
char* UTF8_strchr( unsigned char* utf8_text, unsigned short unicode );
/** 
 * @brief 在utf8字符串中查找另一个字符串
 * 
 * @param utf8_text 指向的源utf8字符串
 * @param utf8_child_str 查找的utf8字符串
 * 
 * @return 查找成功,返回utf8_child_str在utf8_text第一次出现的位置;否则,返回NULL
 */
char* UTF8_strstr( unsigned char* utf8_text, unsigned char* utf8_child_str );
/** 
 * @brief 比较两个utf8字符串
 * 
 * @param utf8_text1 指向的第一个字符串
 * @param utf8_text2 指向的第二个字符串
 * 
 * @return 若两字符串相同,返回0;若utf8_text1大于utf8_text2,则返回大于0的值;若utf8_text1小于utf8_text2,则返回小于0的值
 */
int UTF8_strcmp( unsigned char* utf8_text1, unsigned char* utf8_text2 );
/** 
 * @brief 复制字符串
 * 
 * @param utf8_dest 指向的目的缓冲区
 * @param utf8_src 指向的源字符串
 * 
 * @return 成功,返回目的缓冲区的地址;否则,返回NULL
 */
char* UTF8_strcpy( char* utf8_dest, char* utf8_src );
/** 
 * @brief 字符串连接
 * 
 * @param utf8_dest 指向的目的缓冲区
 * @param utf8_src 指向的源字符串
 * 
 * @return 成功连接,返回目的缓冲区的地址;否则,返回NULL
 */
char* UTF8_strcat( char* utf8_dest, char* utf8_src );
/** 
 * @brief 复制字符串的N个字节
 * 
 * @param utf8_dest 指向的目的缓冲区
 * @param utf8_src 指向的源字符串
 * @param len 复制的字节数目
 * 
 * @return 成功,返回目的缓冲区的地址;否则,返回NULL
 */
char* UTF8_strncpy( char* utf8_dest, char* utf8_src, int len );


/** 
 * @brief 在一个字符串中查找unicode字符,只要找到一个就返回找到的位置,是UTF8_strchr的扩充
 * 
 * @param utf8_text 指向的源utf8字符串
 * @param unicode 查找的unicode字符
 * @param count 
 * 
 * @return 查找成功,返回找到的位置;否则,返回NULL
 */
char* UTF8_FindMultiChar( unsigned char* utf8_text, unsigned short* unicode, int count );

/** 
 * @brief 与UTF8_FindMultiChar的区别是,它要指定字符串的长度
 * 
 * @param utf8_text 指向的源utf8字符串
 * @param maxlen 源utf8字符串的长度
 * @param unicode 查找的unicode字符
 * @param count 
 * 
 * @return 查找成功,返回找到的位置;否则,返回NULL
 */
char* UTF8_FindMultiCharEx( unsigned char* utf8_text, int maxlen, unsigned short* unicode, int count );

/** 
 * @brief 去掉行末指定的字符,返回字符起始位置
 * 
 * @param utf8_text 指向的源utf8字符串
 * @param len 源utf8字符串的长度
 * @param unicode 欲要去掉的unicode字符
 * @param count 
 * 
 * @return 成功,返回去掉后的字符串;否则,返回NULL
 */
char* UTF8_ExceptRChar( unsigned char* utf8_text, int len, unsigned short* unicode, int count );

#endif	/*__UTF8_H__*/

