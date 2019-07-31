/**
 * CONTENT: 对文本格式进行分析
 * HISTORY：
 *		[2005-10-18] chenkai created
 */
#include "swapi.h" 
#include "swtxtparser.h"
#include "swlog.h"

/* 分析是不是一个有效的点分十进制IP地址,本函数不会判断是否时合法的IP地址类 */
bool sw_txtparser_is_address( char* buf )
{
	/* 实际上在internet上的网络ip地址表达式标准:a,a.b,a.b.c,a.b.c.d（可8，10，16进制随机组合) */
	/* !inet_aton(dst->hostname, NULL)用此函数判断更为准确,那天如果有要就需改为此函数判断 */
	if ( buf == NULL )
		return false;
	int i, j, k;
	int sip;
	j = i = 0 ;
	k = 0;
	while (k < 4)
	{
		sip = 0;
		while (buf[i] != '.' && buf[i] != '\0')
		{
			if ( 2 < (i-j) || buf[i] < '0' || '9' < buf[i] )
				return false;
			sip = sip * 10 + buf[i] - '0';
			i++;
		}
		if ( j == i || sip > 255 )/* 没有数据.9..0或者大于255 */
			return false;
		i++;
		j = i;
		++k;
	}
	return (k == 4 && buf[i-2] != '.' && buf[i-1] == '\0') ? true : false;
}

/* 提取等式的左右两边的字符串 */
char* equation_parse( char* equations,char** pleft,char** pright )
{
	char* bptr = equations;
/* 	去掉空格 */
	ADV_SPACE( bptr );
/* 	取得等式的左边 */
		*pleft = bptr; 	
/* 	找等于号 */
	while( *bptr !='\0' && *bptr !=';' && *bptr !='&' )  
	{
		if( *bptr == '=' )
		 	break;
		bptr++;
	}
/* 	找到等于号,取等式右边 */
	if( *bptr == '=' )
	{
		bptr++;
/* 		去掉空格 */
		ADV_SPACE( bptr );
		*pright = bptr;
	}	
	else
		*pright = NULL;
	
/* 	找到下一个equatio的开始 */
	while ( *bptr != '\0' && *bptr != ';' && *bptr != '&' ) 
		bptr++;
		
	if ( *bptr == ';' || *bptr == '&' ) 
		bptr++;
	
	return bptr;
}

/* 提取等式的左右两边的字符串 */
char* equation_parse_as_line( char* equations,char** pleft,char** pright )
{
	char* bptr = equations;

	//去掉特殊字符和空格
	while( *bptr !='\0' && ( *bptr =='\r' || *bptr =='\n' || *bptr ==' ' ) )
		bptr++;
	
	//取得等式的左边
	*pleft = bptr; 	
	//找等于号
	while( *bptr !='\0' && *bptr !='\r' && *bptr !='\n' )  
	{
		if( *bptr == '=' )
		 	break;
		bptr++;
	}
	//找到等于号,取等式右边
	if( *bptr == '=' )
	{
		bptr++;
		//去掉空格
		while( *bptr == ' ' )
			bptr ++;
		*pright = bptr;
	}	
	else
		*pright = NULL;
	
	//找到下一个equatio的开始
	while ( *bptr != '\0' && *bptr !='\r' && *bptr !='\n' ) 
		bptr++;

	return bptr;
}

static unsigned char to_hex ( char *ptr )
{
	if ( isdigit( *ptr ) ) 
	{
		return ( *ptr - '0' );
	}
	return ( tolower( *ptr ) - 'a' + 10 );
}

/* 把数字字符串转化为16进制数组 */
int  txt2hex( const char* string, int length,uint8_t* binary,int binsize )
{
	char* iptr = NULL;
	int len;
	uint8_t* bptr = NULL;
	int ret = -1;
	
	iptr = ( char* )string;
	len=0;
	while ( isxdigit( *iptr ) && len<length ) 
	{
		iptr++;
		len++;
	}
	len >>= 1;
	if ( len == 0 )
	{
		return -1;
	}
	
	if( binsize < len )
	{
		return -1;
	}

	bptr = binary;

	ret = len;  
	iptr =( char* )string; 
	while ( len > 0 ) 
	{
		*bptr++ = ( to_hex( iptr ) << 4 ) | ( to_hex( iptr + 1 ) );
		iptr+=2;
		len--;
	}
	return ret;
}
/* 是否是一个整数 */
bool sw_txtparser_is_int( char* srcint )
{
	unsigned long long aint = 0;
	if ( *srcint == '-' || *srcint == '+' )
		srcint++;
	while (*srcint !='\0')
	{
		if ( *srcint < '0' || '9' < *srcint )
			return false;
		aint = aint * 10 + *srcint - '0';
		if ( aint > 0x0ffffffff )
			return false;
		srcint++;
	}
	return true;
}
/* 参数有效性检查 是否是Mac */
bool sw_txtparser_is_mac_address( char* src_mac )
{
	int i = 0;
	if( strlen( src_mac ) == 17 )
	{
		int flag = 2;
		for( i = 0; i<17; i++ )
		{
			if( flag > 0 )
			{
				if( src_mac[i] < '0' || 'f' < src_mac[i] )
					return false;
			}
			else
			{
				if( src_mac[i] != ':' )
					return false;
				flag = 2;
				continue;
			}
			flag --;
		}
		return true;
	}

	return false;
}
/* 参数有效性检查 是否是带宽 */
bool sw_txtparser_is_band( char* src_band )
{
	int i;
	char bandname[][16] = 
		{
			"256k",		//0x0   
			"512k",		//0x1
			"1M",		//0x2
			"2M",		//0x3
			"5M",		//0x4
			"10M",		//0x5
			"20M",		//0x6
			"50M",		//0x7
			"Unlimited",//0x8   //带宽控制不起作用
			"Enable",	//0x9
			"Disable"	//0xa
		};

	for( i = 0; i < (int)(sizeof( bandname ) / sizeof( bandname[0] ) ); i++ )
	{
		if( strcasecmp( src_band, bandname[i] ) == 0 )
			return true;
	}
	return false;
}

/* 参数有效性检查 是否是一个port(是10进制)0080也算正常的port */
bool sw_txtparser_is_port( char* src_port )
{
	unsigned int port = 0;
	while(*src_port != '\0')
	{
		if ( *src_port < '0' || '9' < *src_port )
			return false;
		port = port * 10 + *src_port - '0';
		if ( port > 0xffff)
			return false;
		src_port++;
	}
	return true;
}

/* 参数有效性检查 是否是音量值 */
bool sw_txtparser_is_vol( char* src_val )
{
	int vol = 0;
	while(*src_val != '\0')
	{
		if ( *src_val < '0' || '9' < *src_val )
			return false;
		vol = vol * 10 + *src_val - '0';
		if ( vol > 30 )
			return false;
		src_val++;
	}
	return true;
}

/* 参数有效性检查 是否是netmode */
bool sw_txtparser_is_net_mode( char* src_net_mode )
{
	if( strcasecmp( src_net_mode,"static" ) == 0 || strcasecmp( src_net_mode,"dhcp" ) == 0
		|| strcasecmp( src_net_mode,"auto-static" ) == 0 || strcasecmp( src_net_mode,"pppoe" )== 0 
		|| strcasecmp( src_net_mode, "wifidhcp" ) == 0 || strcasecmp( src_net_mode, "wifipppoe") == 0
		|| strcasecmp( src_net_mode, "wifistatic") == 0
		)
		return true;
	else
		return false;
}
