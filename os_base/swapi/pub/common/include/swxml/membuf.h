#ifndef _XML_MEMBUF_H
#define _XML_MEMBUF_H

#define MINVAL( a, b ) ( (a) < (b) ? (a) : (b) )
#define MAXVAL( a, b ) ( (a) > (b) ? (a) : (b) )

#define MEMBUF_DEF_SIZE_INC		20

typedef struct
{
	char	*buf;	
	size_t	length;
	size_t	capacity;
	size_t	size_inc;
} xml_membuf;

/** 
 * @brief 
 * 
 * @param m 
 */
void xml_membuf_init(xml_membuf *m);

/** 
 * @brief 
 * 
 * @param m 
 */
void xml_membuf_destroy(xml_membuf *m);

/** 
 * @brief 
 * 
 * @param m 
 * @param buf 
 * @param buf_len 
 * 
 * @return 
 */
int xml_membuf_assign(xml_membuf *m, const void *buf, size_t buf_len );

/** 
 * @brief 
 * 
 * @param m 
 * @param c_str 
 * 
 * @return 
 */
int xml_membuf_assign_str(xml_membuf *m, const char *c_str );

/** 
 * @brief 
 * 
 * @param m 
 * @param buf 
 * 
 * @return 
 */
int xml_membuf_append(xml_membuf *m, const void *buf);

/** 
 * @brief 
 * 
 * @param m 
 * @param c_str 
 * 
 * @return 
 */
int xml_membuf_append_str(xml_membuf *m, const char *c_str);

/** 
 * @brief 
 * 
 * @param m 
 * @param buf 
 * @param buf_len 
 * @param index 
 * 
 * @return 
 */
int xml_membuf_insert(xml_membuf *m, const void* buf, size_t buf_len, int index );

#endif
