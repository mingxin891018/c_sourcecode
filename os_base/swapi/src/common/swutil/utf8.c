#include "swapi.h"
#include "utf8.h"

/*获取UTF8的当个字节总长度*/
int UTF8_CharLen(unsigned char* utf8_text)
{
	if( (utf8_text[0] & 0x80) == 0 )		/*  U+0000 - U+007e */
		return 1;
	if( (utf8_text[0] & 0xf0) == 0xe0 )		/* 16位字 U+0800 - U+ffff */
	{
		if( (utf8_text[1] & 0xc0) == 0x80 && (utf8_text[2] & 0xc0) == 0x80 )
		{
			return 3;
		}
	}
	else if( (utf8_text[0] & 0xe0) == 0xc0 ) /*  U+0080 - U+07ff */
	{
		if( (utf8_text[1] & 0xc0) == 0x80 )
		{
			return 2;
		}
	}
	return 0;
}
/* UTF-8转Unicode，结果在unicode_text中，返回实际使用的utf8_code的个数。如果返回0，要注意排除错误。*/
int UTF8_Unicode( unsigned char* utf8_text, unsigned short* unicode_text )
{
	unsigned char *p = (unsigned char*)utf8_text;
	int num = 0;
	
	if( (p[0] & 0xf0) == 0xe0 )		/* 16位字 U+0800 - U+ffff */
	{
		if( (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80 )
		{
			*unicode_text = ( (p[0]<<12) & 0xf000 ) | ( (p[1]<<6) & 0x0fc0 ) | (p[2] & 0x3f);
			num = 3;
		}
	}
	else if( (p[0] & 0xe0) == 0xc0 ) /*  U+0080 - U+07ff */
	{
		if( (p[1] & 0xc0) == 0x80 )
		{		
			*unicode_text = ( (p[0] << 6) & 0x07c0 ) | (p[1] & 0x3f);
			num = 2;
		}
	}
	else if( (p[0] & 0x80) == 0 )		/*  U+0000 - U+007e */
	{
		*unicode_text = p[0];
		num = 1;
	}
	else
	{
		/* 错误编码 */
	}
	return num;
}

/*拷贝单个UTF8数据,返回拷贝的数据个数*/
int UTF8_charcpy(char *utf8_dest, char* utf8_src)
{
	if( (utf8_src[0] & 0x80) == 0 )		/*  U+0000 - U+007e */
	{
		*utf8_dest = *utf8_src;
		return 1;
	}
	if( (utf8_src[0] & 0xf0) == 0xe0 )		/* 16位字 U+0800 - U+ffff */
	{
		if( (utf8_src[1] & 0xc0) == 0x80 && (utf8_src[2] & 0xc0) == 0x80 )
		{
			*utf8_dest++ = *utf8_src++;
			*utf8_dest++ = *utf8_src++;
			*utf8_dest = *utf8_src;
			return 3;
		}
	}
	else if( (utf8_src[0] & 0xe0) == 0xc0 ) /*  U+0080 - U+07ff */
	{
		if( (utf8_src[1] & 0xc0) == 0x80 )
		{
			*utf8_dest++ = *utf8_src++;
			*utf8_dest = *utf8_src;
			return 2;
		}
	}
	return 0;	
}

int UTF8_strlen(  unsigned char* utf8_text )
{
	int i;
	int num;
	unsigned short tmp_unicode;
	for( i=0; utf8_text[i]; i += num )
	{
		num = UTF8_Unicode( utf8_text+i, &tmp_unicode );
		if( num == 0 )
			break;
	}
	return i;
}


/* 查找一个指定的字符 */
char* UTF8_strchr( unsigned char* utf8_text, unsigned short unicode )
{
	int i;
	int num;
	unsigned short tmp_unicode;
	for( i=0; utf8_text[i]; i += num )
	{
		num = UTF8_Unicode( utf8_text+i, &tmp_unicode );
		if( num==0 )
			break;
		else if( tmp_unicode == unicode )
			return (char*)utf8_text+i;
	}
	return NULL;
}


char* UTF8_strstr( unsigned char* utf8_text, unsigned char* utf8_child_str )
{
	int i;
	int num, len;
	unsigned short tmp_unicode;

	len = UTF8_strlen( utf8_child_str );
	if( len == 0 )
		return NULL;

	for( i=0; utf8_text[i]; i += num )
	{
		if( !memcmp( utf8_text+i, utf8_child_str, len ) )
			return (char*)utf8_text+i;
		num = UTF8_Unicode( utf8_text+i, &tmp_unicode );
		if( num==0 )
			break;
	}
	return NULL;
}


int UTF8_strcmp( unsigned char* utf8_text1, unsigned char* utf8_text2 )
{
	int len = UTF8_strlen( utf8_text1 );
	return memcmp( utf8_text1, utf8_text2, len+1 );
}


char* UTF8_strcpy( char* utf8_dest, char* utf8_src )
{
	int len = UTF8_strlen( (unsigned char*)utf8_src );
	memcpy( utf8_dest, utf8_src, len );//实现的是UTF8_strcpy
	utf8_dest[len] = 0;
	return utf8_dest;
}


char* UTF8_strcat( char* utf8_dest, char* utf8_src )
{
	int len1 = UTF8_strlen( (unsigned char*)utf8_dest );
	int len2 = UTF8_strlen( (unsigned char*)utf8_src );
	memcpy( utf8_dest+len1, utf8_src, len2 );//实现的是UTF8_strcat
	utf8_dest[len1+len2] = 0;
	return utf8_dest;
}

char* UTF8_strncpy( char* utf8_dest, char* utf8_src, int len )
{
	if( len>0 )
	{
		int i = 0;
		int num = 0;
		unsigned short tmp_unicode;
		while( i<len && utf8_src[i] )
		{
			num = UTF8_Unicode( (unsigned char*)utf8_src+i, &tmp_unicode );
			if( num==0 )
				break;
			else
				i += num;
		}
		if( i>len )
			i -= num;
		memcpy( utf8_dest, utf8_src, i );//前面有对i进行长度检测
		if( i<len )
			utf8_dest[i] = 0;
	}
	return utf8_dest;
}


char* UTF8_FindMultiChar( unsigned char* utf8_text, unsigned short* unicode, int count )
{
	int i, j;
	int num;
	unsigned short tmp_unicode;
	for( i=0; utf8_text[i]; i+=num )
	{
		num = UTF8_Unicode( utf8_text+i, &tmp_unicode );
		if( num==0 )
			break;
		for( j=0; j<count; j++ )
		{
			if( unicode[j] == tmp_unicode )
				return (char*)utf8_text + i;
		}
	}
	return NULL;
}





char* UTF8_FindMultiCharEx( unsigned char* utf8_text, int maxlen, unsigned short* unicode, int count )
{
	int i, j;
	int num;
	unsigned short tmp_unicode;
	for( i=0; i<maxlen && utf8_text[i]; i+=num )
	{
		num = UTF8_Unicode( utf8_text+i, &tmp_unicode );
		if( num==0 )
			break;
		for( j=0; j<count; j++ )
		{
			if( unicode[j] == tmp_unicode )
				return (char*)utf8_text + i;
		}
	}
	return NULL;
}


char* UTF8_ExceptRChar( unsigned char* utf8_text, int len, unsigned short* unicode, int count )
{
	int i, j;
	int num, n;
	unsigned short tmp_unicode;

	n = 0;
	for( i=0; i<len; i+=num )
	{
		num = UTF8_Unicode( utf8_text+i, &tmp_unicode );
		if( num==0 )
			return NULL;
		for( j=0; j<count; j++ )
		{
			if( tmp_unicode == unicode[j] )
				break;
		}
		if( j == count )
			n = i + num;
	}
	if( n > len )
		n = len;
	return (char*)utf8_text + n;
}

