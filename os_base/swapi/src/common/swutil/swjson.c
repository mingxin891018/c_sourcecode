#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "swjson.h"


static int decode_value( const char* str, int len, sw_json_t* pvalue, char* buf, int* mem_tail );

#define IS_ALPHA(ch) (('0'<=ch && ch<='9') || ('a'<=ch && ch<='z') || ('A'<=ch && ch<='Z') || ch=='-' || ch=='_' || ch=='@' || ch=='$')
#define IS_CTRL(ch) (ch=='\"' || ch=='\\' || ch=='/' || ch=='b' || ch=='f' || ch=='n' || ch=='r' || ch=='\t')


/***
 *   从 buf 中分配 size 大小的缓冲区。
 */
static void* buf_alloc( int size, char* buf, int* mem_tail )
{
	int len = (size+3)&~3;
	if( *mem_tail < len )
		return NULL;
	*mem_tail -= len;
	return buf + *mem_tail;
}


/***
 *   复制字符串，内存从 buf 中分配。
 *   如果字符串头、尾有引号，则去掉引号。如果字符串内有转义字符，会转义字符
 */
static char* buf_strdup( const char* str, int len, char* buf, int* mem_tail )
{
	if( str[0] == '\"' || str[0] == '\'' )
	{
		if( len < 2 || str[len-1] != str[0] )
			return NULL;
		str ++;
		len -= 2;
	}
	char* s = (char*)buf_alloc( len+1, buf, mem_tail );
	if( s )
	{
		int i, n = 0;
		for( i=0; i<len; i++ )
		{
			if( str[i] == '\\' && IS_CTRL(str[i+1]) )
			{
				if( str[i+1] == 'b' )
					s[n] = '\b';
				else if( str[i+1] == 'f' )
					s[n] = '\f';
				else if( str[i+1] == 'n' )
					s[n] = '\n';
				else if( str[i+1] == 'r' )
					s[n] = '\r';
				else if( str[i+1] == 't' )
					s[n] = '\t';
				else
					s[n] = str[i+1];
				i++;
			}
			else
			{
				s[n] = str[i];
			}
			n++;
		}
		s[n] = 0;
	}
	return s;
}


/***
 *   获取一个单元的长度。此单元可以是以引号为定界符的字符串，也可以是无引号定界符的字符串
 */
static int get_string_length( const char* str, int length )
{
	char ch = 0;
	const char* p = str;
	if( *p == '\"' || *p == '\'' )
	{
		ch = *p;
		p++;
		while( p-str < length )
		{
			if( *p == '\\' )
			{
				if( IS_CTRL(p[1]) )
					p++;
			}
			else if( *p == ch )
				return p+1-str;
			p++;
		}
		return -1;
	}
	else
	{
		while( p-str < length && IS_ALPHA(*p) )
			p++;
		return p-str;
	}
}


/***
 *   获取一个 json 的长度，此json 以 {} 或 [] 为定界符
 */
static int get_json_length( const char* str, int length )
{
	int n = 0;
	char ch = 0, th = 0;
	const char *p = str;
	if( *p == '[' || *p == '{' )
	{
		ch = *p;
		p++;
		th = ch=='[' ? ']' : '}';
		
		while( p-str < length )
		{
			if( *p == '\\' )
			{
				if( IS_CTRL(p[1]) )
					p++;
			}
			else if( *p == '\"' || *p == '\'' )
			{
				int vlen = get_string_length(p, str+length-p);
				if( vlen <= 0 )
					return -1;
				p += vlen-1;
			}
			else if( *p == ch )
				n++;
			else if( *p == th )
			{
				if( n==0 )
					return p+1 - str;
				n--;
			}
			p++;
		}
		return -1;
	}
	else
	{
		return 0;
	}
}




/***
 *   解析一个以 {} 为边界的 json 串，结果为一个 json_object_t 对象
 */
static int decode_object(const char* str, int len, json_object_t** pjobj, char* buf, int* mem_tail)
{
	const char* p = str;
	int i, n, mtail = *mem_tail;
	int maxcount = 16384, ncnt = 0;
	struct name_value_t
	{
		const char* name;
		int nlen;
		const char* value;
		int vlen;
	};
	struct name_value_t* nv;
	
	if( *p != '{' )
		return -1;

	if( maxcount > len/4 )
		maxcount = len/4;
	if( maxcount <= 0 )
	{
		*pjobj = NULL;
		return len;
	}
	nv = (struct name_value_t*)malloc( maxcount * sizeof(struct name_value_t) );
	if( nv == NULL )
		return -1;
	
	//将以 {} 分隔的范围内的元素分成 name:value 对，存放到 struct name_value_t 中
	p++;
	while( p-str < len )
	{
		//获取 name 的长度
		while( p-str < len && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n') )
			p++;
		if( *p == '}' )
		{
			p++;
			break;
		}
		n = get_string_length( p, str+len - p );
		if( n <= 0 )
			goto ERR_EXIT;	//缺少 name
		nv[ncnt].name = p;
		nv[ncnt].nlen = n;
		p += n;
		
		//跳过冒号
		i = 0;
		while( p-str < len && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n' || *p==':') )
		{
			if( *p == ':' )
				i++;
			p++;
		}
		if( i != 1 )		//在 name:value 对的中间缺少冒号
			goto ERR_EXIT;
		
		//获取 value 的长度
		if( *p == '[' || *p == '{' )
			n = get_json_length( p, str+len - p );
		else
			n = get_string_length( p, str+len - p );
		if( n <= 0 )
			goto ERR_EXIT;
		nv[ncnt].value = p;
		nv[ncnt].vlen = n;
		p += n;
		ncnt ++;
		if( ncnt >= maxcount )
			break;
		
		//跳过逗号
		i = 0;
		while( p-str < len && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n' || *p==',') )
		{
			if( *p == ',' )
				i++;
			p++;
		}
		if( i != 1 && *p != '}' )	//在 name:value 对的后面缺少逗号
			goto ERR_EXIT;
	}
	
	//解析 nv 数组，其长度为 ncnt
	json_object_t* jobj = (json_object_t*)buf_alloc( sizeof(json_object_t), buf, &mtail );
	if( jobj == NULL )
		goto ERR_EXIT;

	jobj->count = ncnt;	
	jobj->nvs = (json_name_value_t*)buf_alloc( ncnt * sizeof(json_name_value_t), buf, &mtail );
	if( jobj->nvs == NULL )
		goto ERR_EXIT;

	memset( jobj->nvs, 0, ncnt * sizeof(json_name_value_t) );
	for( i=0; i<ncnt; i++ )
	{
		jobj->nvs[i].name = buf_strdup( nv[i].name, nv[i].nlen, buf, &mtail );
		if( jobj->nvs[i].name == NULL )
			goto ERR_EXIT;
		n = decode_value( nv[i].value, nv[i].vlen, &jobj->nvs[i].value, buf, &mtail );
		if( n < 0 )
			goto ERR_EXIT;
	}
	
	free( nv );
	*mem_tail = mtail;
	*pjobj = jobj;
	return p-str;

ERR_EXIT:
	free( nv );
	return -1;
}



/***
 *   解析一个以 [] 为边界的 json 串，结果为一个 json_array_t 对象
 */
static int decode_array(const char* str, int len, json_array_t** pjarr, char* buf, int* mem_tail)
{
	const char* p = str;
	int i, n, mtail = *mem_tail;
	int maxcount = 16384, ncnt = 0;
	struct value_t
	{
		const char* value;
		int vlen;
	};
	struct value_t* vs;
	
	if( *p != '[' )
		return -1;

	if( maxcount > len/2 )
		maxcount = len/2;
	if( maxcount <= 0 )
	{
		*pjarr = NULL;
		return len;
	}
	vs = (struct value_t*)malloc( maxcount * sizeof(struct value_t) );
	if( vs == NULL )
		return -1;

	//将以 [] 分隔的范围内的元素分成一个一个的 value，存放到 struct value_t 中
	p++;
	while( p-str < len )
	{
		while( p-str < len && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n') )
			p++;
		if( *p == ']' )
		{
			p++;
			break;
		}

		//获取 value 的长度
		if( *p == '[' || *p == '{' )
			n = get_json_length( p, str+len - p );
		else
			n = get_string_length( p, str+len - p );
		if( n <= 0 )
			goto ERR_EXIT;
		vs[ncnt].value = p;
		vs[ncnt].vlen = n;
		p += n;
		ncnt ++;
		if( ncnt >= maxcount )
			break;

		//跳过逗号
		i = 0;
		while( p-str < len && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n' || *p==',') )
		{
			if( *p == ',' )
				i++;
			p++;
		}
		if( i != 1 && *p != ']' )	//在 value 的后面缺少逗号
			goto ERR_EXIT;
	}

	//解析 vs 数组，其长度为 ncnt
	json_array_t* jarr = (json_array_t*)buf_alloc( sizeof(json_array_t), buf, &mtail );
	if( jarr == NULL )
		goto ERR_EXIT;

	jarr->count = ncnt;	
	jarr->values = (sw_json_t*)buf_alloc( ncnt * sizeof(sw_json_t), buf, &mtail );
	if( jarr->values == NULL )
		goto ERR_EXIT;

	memset( jarr->values, 0, ncnt * sizeof(sw_json_t) );
	for( i=0; i<ncnt; i++ )
	{
		n = decode_value( vs[i].value, vs[i].vlen, &jarr->values[i], buf, &mtail );
		if( n < 0 )
			goto ERR_EXIT;
	}
	
	free( vs );
	*mem_tail = mtail;
	*pjarr = jarr;
	return p-str;
	
ERR_EXIT:
	free( vs );
	return -1;
}



/***
 *   解析一个 json 值，结果存放到 sw_json_t 对象中
 */
static int decode_value( const char* str, int len, sw_json_t* pvalue, char* buf, int* mem_tail )
{
	int ret;
	if( str[0] == '{' )
	{
		ret = decode_object(str, len, &pvalue->jobj, buf, mem_tail);
		if( ret<0 )
			goto ERR_EXIT;
		pvalue->type = VTYPE_OBJ;
	}
	else if( str[0] == '[' )
	{
		ret = decode_array(str, len, &pvalue->jarr, buf, mem_tail);
		if( ret<0 )
			goto ERR_EXIT;
		pvalue->type = VTYPE_ARRAY;
	}
	else
	{
		pvalue->str = buf_strdup( str, len, buf, mem_tail );
		if( pvalue->str == NULL )
			goto ERR_EXIT;
		pvalue->type = VTYPE_STR;
	}

	return 0;

ERR_EXIT:
	return -1;
}


/***
 *   解析 json 串，结果是一个 sw_json_t 对象指针
 * @param str: json 串
 * @param length: str 长度
 * @param buf: 解析 json 串使用的缓冲区，存放解析结果
 * @param size: buf 的大小
 * @return: 解析结果对象的指针，NULL 表示解析失败
 * 注：解析结果存放在 buf 中，因此不需要单独销毁
 */
sw_json_t* sw_json_decode( const char* str, int length, void* buf, int size )
{
	const char* p = str;
	while( p-str < length && (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n') )
		p++;
	
	sw_json_t* json = (sw_json_t*)buf;
	int mtail = size - sizeof(sw_json_t);
	if( mtail <= 0 )
		return NULL;

	int ret = decode_value( p, str+length-p, json, (char*)buf+sizeof(sw_json_t), &mtail );
	if( ret < 0 )
		return NULL;
	return json;
}
