/************************************************************
* AUTHOR: lijian / huanghuaming / chenkai
* CONTENT: 实现string的扩展函数
* NOTE:	
* HISTORY:
* [2005-9-16] created
***********************************************************/
#include "swapi.h"
#include "string_ext.h"


/* 不区分大小写比较字符串的前n个字符 */
int xstrncasecmp(const char* s1, const char* s2, size_t n)
{
	int i=0, ret=0;
	
	while( s1[i] != 0 && s2[i] != 0 && i < (int)n )
	{
		ret = toupper(s1[i]) - toupper(s2[i]);
		if( ret != 0 )
			break;
		i++;
	}
	if( i < (int)n )
		ret = toupper(s1[i]) - toupper(s2[i]);

	if( ret == 0 )
		return 0;
	else if( ret < 0 )
		return -1;
	else
		return 1;
	
	return 0;
}


/* 不区分大小比较字符串 */
int xstrcasecmp(const char *s1, const char *s2)
{
	int i=0, ret=0;

	while( s1[i] != 0 && s2[i] != 0 )
	{
		ret = toupper(s1[i]) - toupper(s2[i]);
		if( ret != 0 )
			break;
		i++;
	}
	ret = toupper(s1[i]) - toupper(s2[i]);

	if( ret == 0 )
		return 0;
	else if( ret < 0 )
		return -1;
	else
		return 1;
}


/* 把字符串按delim定义的字符分割，并返回分割后的第一个串 */
char* xstrsep(char** stringp, const char *delim)
{
	char ch ='\0';
	char* pRet = NULL;
	if( *stringp == NULL )
		return NULL;
	
	ch = *(*stringp);
	pRet =*stringp;
	while(ch != '\0')
	{
		if( strchr(delim,ch) != NULL )
		{
			*(*stringp) = '\0';
			(*stringp)++;
			ch = *(*stringp);
			while( ch != '\0' && strchr(delim,ch) != NULL )
			{
				(*stringp)++;
				ch = *(*stringp);
			}
			break;
		}
		else
			(*stringp)++;
		ch = *(*stringp);
	}
	if( ch == '\0' )
		*stringp = NULL;
	
	return pRet;
}

/* 把最多size个字符输入到字符串 */
int xsnprintf(char *str, size_t size, const char *format, ...)
{
	int nRet=0;
	va_list ap;
	
	va_start(ap, format);
#ifdef WIN32
	nRet = _vsnprintf(str,size,format, ap );
#else
	nRet = vsnprintf(str,size,format, ap );
#endif
	va_end(ap);
	if(nRet > size)
		nRet = size;
	
	return nRet;
}

int xvsnprintf(char *s, size_t buf_size, const char *format, va_list ap)
{
	int nRet = 0;
#ifdef WIN32
	nRet = _vsnprintf(s,buf_size,format,ap);
#else
	nRet = vsnprintf(s,buf_size,format,ap);
#endif
	if(nRet > buf_size)
		nRet = buf_size;
	return nRet;
	
}




/* 提取字段值 */
char *xstrgetval( char *buf, char *name, char *value, int valuelen )
{
	char *s, *e, *p;
	int len = strlen(buf );

	memset( value, 0, valuelen );

	/* 分析版本信息文件 */
	p = s = buf;
	while( p < buf+len )
	{
		/* 去掉开始的空格 */
		while( *p == ' ' ||  *p == ',' ||  *p == '\t' || *p == '\r' || *p == '\n' )
			p++;

		/* 记录行开始 */
		s = p;

		/* 找到行结束 */
		while( *p != '\r' && *p != '\n' && *p != '\0' )
			p++;
		e = p;

		/* 找到需要的字段 */
		if( xstrncasecmp( s, name, strlen(name) ) == 0 )
		{
			/* 找到名称结束 */
			p = s;
			while( *p != ':' &&  *p != '=' &&  *p != '\t' && *p != '\r' && *p != '\n' && *p != '\0' )
				p++;
			if( *p == 0 || e <= p )
				goto NEXT_LINE;

			/* 找到值开始 */
			p++;
			while( *p == ':' ||  *p == '=' ||  *p == ',' ||  *p == '\t' || *p == '\r' || *p == '\n' )
				p++;
			strncpy( value, p, valuelen-1<e-p ? valuelen-1:e-p );/* 入口处已经清零value了,切e-p肯定>0也可使用memcpy或者memmove */
			return value;
		}
NEXT_LINE:
		p = e + 1;
	}
	return NULL;
}

/*
 @brief:判断输入的字符串是否是数字
 */
int xstrisdigit(char* str)
{
  if( str == NULL )
    return -1;
  for( ; *str!=0; str++)
  {
    if( isdigit(*str) == 0 )
      return -1;
  }
  return 0;
}

