/*
 * @file swlog.c
 * @brief The file defines the interfaces to manipulate logging information.
 * @author 	qushihui
 * @version 	%I%, %G%
 * @history
 * 			2010-07-01 qushihui created
 *			2010-12-03 chenkai  optimized
 */

#include "swapi.h"
#include "swudp.h"
#include "swfile.h"
#include "swsignal.h"
#include "swlog.h"
#include "swthrd.h"

#ifndef WIN32
#include <sys/un.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#endif


#define LOG_BUF_INDEX_MASK		0x7	//7个数组，0x3,0xf
#define LOG_TARGETS_MAX		4       //目标数组最大容量
#define LOG_MODS_MAX		64		//最多允许输出日志的模块数目64
#define LOG_BUF_SIZE 		1024	//每条日志最大的大小,需要确保本buf大小小于ftp日志申请的空间大小
#define LOG_HEADER_SIZE 	128	//每条日志头信息最大的大小
#define LOG_MODSINFO_LENGTH	 1024	//mods 列表信息长度
#define LOG_TARGETSINFO_LENGTH	 1024	//tagets 列表信息长度
#define LOG_LOCAL_SOCKET_FILE    "/tmp/.logsocket"

//把四个字符转化成一个uin32_t型数据
#ifdef WORDS_BIGENDIAN
#define SW_FOURCC( a, b, c, d ) \
        ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) \
        | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#else
#define SW_FOURCC( a, b, c, d )   (((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

//日志输出目标的类型
typedef enum
{
	LOG_TARGET_NULL = 0x00,
	LOG_TARGET_CONSOLE,
	LOG_TARGET_FILE,
    LOG_TARGET_LOCALSOCK,
	LOG_TARGET_MAX
}target_type_t;

//日志文件的状态
typedef enum
{
	LOG_FP_OFF =0x00,
	LOG_FP_OPEN,
	LOG_FP_ON,
	LOG_FP_FULL,
	LOG_FP_CLOSE,
	LOG_FP_MAX
}log_fp_state_t;

//日志输出目标
typedef struct 
{
	target_type_t  type;
	char  target_org[256];				//目标路径
	union
	{
		struct 	log_file_t
		{
			FILE* fp1;					//日志文件句柄
			uint32_t fp1_size;			//日志文件当前数据大小
			log_fp_state_t fp1_state;	//日志文件状态
			int fp1_fullcount;			//日志文件处于文件满时的计数
			FILE* fp2;					//日志文件句柄	
			uint32_t fp2_size;			//日志文件当前数据大小	
			log_fp_state_t fp2_state;	//日志文件状态
			int fp2_fullcount;			//日志文件处于文件满时的计数
			uint32_t maxsize;			//日志文件最大大小
			int count;					//日志文件的计数,用于生成日志文件名
			char path[256];				//日志文件的路径
		}logfile;

		struct log_udp_t
		{
			uint32_t ip;
			uint16_t port;

		}logudp;

#ifndef WIN32
        struct log_localsock_t
        {
            int    sock;
            struct sockaddr_un sock_un;
        }loglocalsock;
#endif
	}log_union;	
}target_t;

//打印等级，默认OFF
static int m_level = LOG_LEVEL_OFF; 
//输出日志类型，默认全部输出
static int m_type = LOG_TYPE_ALL; 
//日志输出目标数组
static target_t m_targets_list[LOG_TARGETS_MAX];
//日志输出目标信息
static char m_targets_info[LOG_TARGETSINFO_LENGTH];
//日志输出目标数目										
static int m_targets_num = 0;	
//模块开关标志
static bool m_allmods_is_allowed = false;
//允许输出日志的模块信息
static char m_mods_info[LOG_MODSINFO_LENGTH];
//允许输出日志的模块列表
static uint32_t m_mods_list[LOG_MODS_MAX];
//不允许输出日志的模块列表
static uint32_t m_disable_mods_list[LOG_MODS_MAX];
//允许输出日志的模块书目
static int m_mods_num = 0;
//日志输出等级信息
static const char* m_level_info[] = { "ALL", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" }; 
//日志buf
static char  m_logbuf[LOG_BUF_INDEX_MASK+1][LOG_BUF_SIZE+LOG_HEADER_SIZE];//头预留给logheader输出时用
static char  m_sw_log_header[LOG_BUF_INDEX_MASK+1][LOG_HEADER_SIZE];	
//日志buf索引
static int m_logbuf_index=0;

/**
 *@brief 把日志模块的名字转成数值
 */
static uint32_t calc_imod(const char* mod);
/**
 *@brief 查找mod是否存在
 */
static int find_imod(uint32_t imod);
/**
 *@brief 判断模块是否允许输出日志
 */
static bool log_mod_is_allowed(const char* mod);
/**
 *@brief 输出日志
 */
static int log_output(char* logbuf,int size, char* sw_head, int sw_size);
/**
 *@brief 打开文件
 */
static FILE* log_open_file(char* path,int count);
/**
 *@brief 查找日志输出目标
 */
static int log_find_target(char* target);
/**
 *@brief 找一个空的日志输出目标，以便存储新的日志输出目标
 */
static int log_find_empty_target( );


static char m_pidinfo[8] = {0};
static char m_syslog_header[128] = {0};

static int syslog_get_local_time(char *tmbuf,int size)
{
	if(tmbuf == NULL)
		return -1;
	time_t now;
	struct tm tm_now;
	time(&now);
	localtime_r(&now, &tm_now);
	int len = sw_snprintf(tmbuf,size,0,size,"%02d-%02d %02d:%02d:%02d",tm_now.tm_mon+1,tm_now.tm_mday,tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
	return len;
}


/** 
 * @brief Initialize the logging API, prepare for logging.
 * 
 * @param level int,  The original level to record the logging infomation. You can use sw_log_set_level() to change it.
 * @param target char*,  The target of all logging infomation.
 * @param mods char*,The modules which allow to ouput log.
 * 
 * @return int , The status of this operation.  0 on success, else on failure.
 */
int sw_log_init( int level, char* targets, char* mods )
{
    int i = 0;
	if( level < LOG_LEVEL_ALL || level > LOG_LEVEL_OFF )
	{
		printf("level error:0-6\n ");
		return -1;
	}

#ifndef WIN32
    for( i = 0; i < LOG_TARGETS_MAX; i++ )
    {
        m_targets_list[i].log_union.loglocalsock.sock = -1;
    }

#endif
	m_level = level;	
	sw_log_set_targets( targets);
	sw_log_set_mods( mods);
	memset(m_pidinfo, 0, sizeof(m_pidinfo));
#if defined(ANDROID)
#elif defined(LINUX)
	//是否需要输出进程号?
	snprintf(m_pidinfo, sizeof(m_pidinfo), "(%d)",getpid());
#endif
	return 0;
}

/** 
 * @brief Release some resources allocated by sw_log_init().
 */
void sw_log_exit()
{
	int i = 0;
	m_level = LOG_LEVEL_OFF ;
	sw_log_clear_alltarget();
	sw_log_clear_allmods();
}

/** 
 * @brief set the logging level
 * 
 * @param level int, the logging level to set.
 */
int sw_log_set_level( int level )
{
	if( level < LOG_LEVEL_ALL )
		m_level = LOG_LEVEL_ALL;
	else if( level > LOG_LEVEL_OFF )
		m_level = LOG_LEVEL_OFF;
	else
		m_level = level;
	return 0;
}


/** 
 * @brief To get the logging level.
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log_get_level()
{
	return m_level;
}

/** 
 * @brief set the logging level
 * 
 * @param level int, the logging level to set.
 */
int sw_log_set_type( int type )
{
	m_type = type;
	return 0;
}

/** 
 * @brief To get the logging type.
 * 
 * @return int , the logging type has been set previously.
 */
int sw_log_get_type()
{
	return m_type;
}

/** 
 * @brief  set the target of the logging info. It will override the original target list.
 * The new target list has only one target.
 * @param target,char*,the target to set,for example console,file:///tmp/disk1/log.log&max_size=5M
 */
int sw_log_set_targets( char* targets )
{	
	char* p =NULL;
	char* q =NULL;
	char target[256];

	if(targets == NULL)
		return -1;

	sw_log_clear_alltarget();
	p = targets;
	while( *p != '\0' )
	{
		q = p;
		while( *q !=',' && *q !='\0')
			q++;			

		if( (q-p) <= (int)(sizeof(target)-1))
		{
			memmove(target,p,q-p);
			target[q-p]='\0';
		}
		else
		{
			memmove(target,p,sizeof(target)-1);
			target[sizeof(target)-1] = '\0';
		}

		sw_log_add_target(target);

		if( *q == '\0')
			break;
		else
			p = q+1;
	}
	return m_targets_num;
}

/** 
 * @brief Add a target to the target list.
 * 
 * @param target,char*,the target to add.for example console,file:///tmp/disk1/log.log&max_size=5M
 *		for:ftp://ftpuser:111111@192.168.0.8/00-11-09-EF-B5-AF when update log swlog modules will pack as
 *			ftp://ftpuser:111111@192.168.0.8/00-11-09-EF-B5-AF_20110517142934.log
 */
int sw_log_add_target( char* target )
{
	int i =0;
	char* p = NULL;
	char* q = NULL;

	if( target == NULL )
		return -1;

	if( log_find_target(target)>=0 )
		return 0;
	
	i = log_find_empty_target();
	if( i < 0 )
	{
		printf("target is full,add target failed...\n");
		return -2;
	}
	target_t *p_target = &(m_targets_list[i]);
	
    memset(p_target->target_org, 0, sizeof(p_target->target_org));
	strlcpy(p_target->target_org,target,sizeof(p_target->target_org) );
	if( strncmp(target,"console",strlen("console")) == 0 )
	{
		p_target->type = LOG_TARGET_CONSOLE;
		m_targets_num ++;
	}
#ifndef WIN32
    else if( strncmp( target, "localsock", strlen("localsock") ) == 0 )
    {
        if( p_target->log_union.loglocalsock.sock < 0 )
        {
            p_target->type = LOG_TARGET_LOCALSOCK;
            p_target->log_union.loglocalsock.sock = socket( AF_UNIX, SOCK_DGRAM, 0 );
            p_target->log_union.loglocalsock.sock_un.sun_family = AF_LOCAL;
            strlcpy( p_target->log_union.loglocalsock.sock_un.sun_path, LOG_LOCAL_SOCKET_FILE, sizeof(p_target->log_union.loglocalsock.sock_un.sun_path));
            m_targets_num ++;
        }
    }
#endif
	else if( strncmp(target,"file://",strlen("file://")) == 0)
	{
		p = strstr(p_target->target_org,"&max_size=");
		if( p )
		{
			p_target->log_union.logfile.maxsize = strtol(p+strlen("&max_size="),&q,0);
			if(q)
			{
				if( *q == 'M' || *q=='m' )
					p_target->log_union.logfile.maxsize *= 1024*1024;
				else if( *q == 'K' || *q=='k' )
					p_target->log_union.logfile.maxsize *= 1024;
	
				//最小文件大小128K
				if( p_target->log_union.logfile.maxsize < 128*1024)
					p_target->log_union.logfile.maxsize = 128*1024;
			}
			*p = '\0';
            memset(p_target->log_union.logfile.path, 0, sizeof(p_target->log_union.logfile.path));
			strlcpy(p_target->log_union.logfile.path,p_target->target_org+strlen("file://"),sizeof(p_target->log_union.logfile.path));
			*p = '&';
		}
		else
		{
			p_target->log_union.logfile.maxsize = 512 *1024;	//缺省文件大小为512K
            memset(p_target->log_union.logfile.path, 0, sizeof(p_target->log_union.logfile.path));
			strlcpy(p_target->log_union.logfile.path,target+strlen("file://"),sizeof(p_target->log_union.logfile.path) );
		}
		p_target->log_union.logfile.count = 0;
		p_target->log_union.logfile.fp1_state = LOG_FP_OPEN;
		p_target->log_union.logfile.fp1=log_open_file(p_target->log_union.logfile.path,p_target->log_union.logfile.count);
		if( p_target->log_union.logfile.fp1 )
		{
			p_target->log_union.logfile.count++;
			if( p_target->log_union.logfile.count >= 4 )
			{	
				p_target->log_union.logfile.count = 0;
			}
			p_target->log_union.logfile.fp1_size = 0;
			p_target->log_union.logfile.fp1_fullcount = 0;
			p_target->log_union.logfile.fp1_state = LOG_FP_ON;
		}
		else
		{
			printf("Open file %s \n failed ...\n",p_target->log_union.logfile.path);			
			p_target->log_union.logfile.fp1_state	= LOG_FP_OFF;
			return -3;
		}
		p_target->log_union.logfile.fp2 = NULL;
		p_target->log_union.logfile.fp2_size = 0;
		p_target->log_union.logfile.fp2_fullcount = 0;
		p_target->log_union.logfile.fp2_state	=LOG_FP_OFF;

		p_target->type = LOG_TARGET_FILE;
		m_targets_num ++;
	}

	//添加target
    m_targets_info[sizeof(m_targets_info) - 1] = '\0';
	if( m_targets_info[0] != '\0')
		strlcat(m_targets_info,",",sizeof(m_targets_info));
	strlcat(m_targets_info,target,sizeof(m_targets_info));
	return 0;
}

/** 
 * @brief Remove a target from the target list.
 * 
 * @param target,char*, the target to del.
 */
int sw_log_del_target( char* target )
{
	target_type_t  type;	
	target_t* p_target = NULL;
	int i = 0;
	char *p = NULL;
	char *q = NULL;

	if( target == NULL )
		return -1;

	i = log_find_target(target);

	if( i >= 0 )
		p_target = &(m_targets_list[i]);

	if( p_target )
	{
		type = p_target->type;
		p_target->type = LOG_TARGET_NULL;
		//让当前的日志输出完
		usleep(50*1000);
		//关闭日志文件
		if( type == LOG_TARGET_FILE )
		{
			if(p_target->log_union.logfile.fp1_state !=  LOG_FP_OFF)
			{
				p_target->log_union.logfile.fp1_state = LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp1)
					fclose(p_target->log_union.logfile.fp1);
				p_target->log_union.logfile.fp1 = NULL;
				p_target->log_union.logfile.fp1_fullcount = 0; 
				p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
			}
			if( p_target->log_union.logfile.fp2_state != LOG_FP_OFF )
			{
				p_target->log_union.logfile.fp2_state=LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp2)
					fclose(p_target->log_union.logfile.fp2);
				p_target->log_union.logfile.fp2 = NULL;
				p_target->log_union.logfile.fp2_fullcount = 0; 
				p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
			}
		}
#ifndef WIN32
        else if( type == LOG_TARGET_LOCALSOCK )
        {
            if( p_target->log_union.loglocalsock.sock >= 0 )
            {
                shutdown( p_target->log_union.loglocalsock.sock, SHUT_RDWR );
                close( p_target->log_union.loglocalsock.sock );
                p_target->log_union.loglocalsock.sock = -1;
            }
        }
#endif
		memset(p_target->target_org,0,sizeof(p_target->target_org));
		m_targets_num --;

		//从targets信息里删除target
		p = strstr(m_targets_info,target);
		if( p )
		{
			q = p;
			while(*q !=',' && *q != '\0' )
				q++;
			if( *q == ',' )
				q++;
			memmove(p,q,strlen(q)+1);
			if( m_targets_info[0] != '\0' && *p == '\0')
			{
				memmove(p-1,p,strlen(p)+1);
			}
		}
	}
	return 0;
}

/** 
 * @brief Remove all targets from the target list.
 */
void sw_log_clear_alltarget()
{
	
	int level = m_level;
	int count = 0;
	int i;
	target_t* p_target = NULL;
	//设置log_level 
	m_level = LOG_LEVEL_OFF;

	//delay 50ms,让没有正在输出的日志输出完
	usleep(50*1000);

	memset(m_targets_info,0,sizeof(m_targets_info));
	//关闭打开的文件句柄
	for( i=0;i<LOG_TARGETS_MAX && count<m_targets_num;i++)
	{
		p_target = &(m_targets_list[i]);
		if( p_target->type == LOG_TARGET_FILE )
		{
			if(p_target->log_union.logfile.fp1_state !=  LOG_FP_OFF)
			{
				p_target->log_union.logfile.fp1_state = LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp1)
					fclose(p_target->log_union.logfile.fp1);
				p_target->log_union.logfile.fp1 = NULL;
				p_target->log_union.logfile.fp1_fullcount = 0; 
				p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
			}
			if( p_target->log_union.logfile.fp2_state != LOG_FP_OFF )
			{
				p_target->log_union.logfile.fp2_state=LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp2)
					fclose(p_target->log_union.logfile.fp2);
				p_target->log_union.logfile.fp2 = NULL;
				p_target->log_union.logfile.fp2_fullcount = 0; 
				p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
			}
		}
#ifndef WIN32
        else if( p_target->type == LOG_TARGET_LOCALSOCK )
        {
            if( p_target->log_union.loglocalsock.sock > 0 )
            {
                shutdown( p_target->log_union.loglocalsock.sock, SHUT_RDWR );
                close( p_target->log_union.loglocalsock.sock );
                p_target->log_union.loglocalsock.sock = -1;
            }
        }
#endif
		if(p_target->type != LOG_TARGET_NULL) 
		{
			count++;
			p_target->type = LOG_TARGET_NULL;
			memset(p_target->target_org,0,sizeof(p_target->target_org));
		}
	}
	m_targets_num = 0;
	m_level = level;
	return ;
}

/** 
 * @brief return the target list.
 *
 * @return the target list
 */
char* sw_log_get_targets()
{
	return m_targets_info;
}


/** 
 * @brief set some modules to output log,it will override thr original log modules
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int sw_log_set_mods( char* mods )
{
	char* p =NULL;
	char* q =NULL;
	char tmod[64];

	if( mods == NULL )
		return -1;

	m_mods_num = 0;
    memset(m_mods_info, 0, sizeof(m_mods_info));
	strlcpy(m_mods_info,mods,sizeof(m_mods_info));	
	memset(m_mods_list,0,sizeof(m_mods_list));
	memset(m_disable_mods_list, 0, sizeof(m_disable_mods_list));
	p = m_mods_info;
	while( *p != '\0' )
	{
		q = p;
		while( *q !=',' && *q !='\0')
		{
			q++;			
		}
		if( (q-p) <= (int)sizeof(tmod)-1 )
		{
			memmove(tmod,p,q-p);
			tmod[q-p]='\0';
		}
		else
		{
			memmove(tmod,p,sizeof(tmod)-1);
			tmod[sizeof(tmod)-1] = '\0';
		}
		m_mods_list[m_mods_num] = calc_imod(tmod);	
		if( m_mods_list[m_mods_num] == SW_FOURCC('a','l','l',0) )
			m_allmods_is_allowed = true;

		m_mods_num++;

		if( *q == '\0')
			break;
		else
			p = q+1;
	}
	return m_mods_num;
}

/** 
 * @brief Add a log module to the log modules list.
 * 
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int sw_log_add_mod( char* mod)
{
	uint32_t imod = 0;
	
	if( mod == NULL )
		return -1;

	imod = calc_imod(mod);

	if( imod == SW_FOURCC('a','l','l',0) )
		m_allmods_is_allowed = true;

	//查找模块是否存在，如果存在直接返回
	if( find_imod(imod)>=0 )
		return 0;
	//如果队列已经满,则增加失败
	if( m_mods_num == LOG_MODS_MAX )
		return -2;
	//增加mod
	if( m_mods_info[0] == '\0')
	{
        m_mods_info[sizeof(m_mods_info) - 1] = '\0';
		strlcat(m_mods_info,mod,sizeof(m_mods_info));
	}
	else
	{
        m_mods_info[sizeof(m_mods_info) - 1] = '\0';
		snprintf(m_mods_info+strlen(m_mods_info),sizeof(m_mods_info)-strlen(m_mods_info),",%s",mod);
	}
	m_mods_list[m_mods_num] = imod;
	m_mods_num ++;
	return 0;
}

/** 
 * @brief Remove a log module from the log modules list.
 * 
 * @param mods,char*, the modules to del.
 */
int sw_log_del_mod( char* mod )
{
	int i =0;
	char* p =  NULL;
	char* q = NULL;
	uint32_t imod =0;
	
	if( mod == NULL )	
		return -1;

	imod = calc_imod( mod );
	
	if( imod == SW_FOURCC('a','l','l',0) )
		m_allmods_is_allowed = false;

	if( (i=find_imod(imod)) <0 )
		return 0;

	m_mods_list[i] = 0;
	m_mods_num--;

	//从mod信息里删除mod
	p = strstr(m_mods_info,mod);
	if( p )
	{
		q = p;
		while(*q !=',' && *q != '\0' )
			q++;
		if( *q == ',' )
			q++;
		memmove(p,q,strlen(q)+1);

		if( m_targets_info[0] != '\0' && *p == '\0')
		{
			memmove(p-1,p,strlen(p)+1);
		}
	}

	return 0;
}

/** 
 * @brief Remove all log modules from the log modules list.
 */
int sw_log_clear_allmods()
{	
	memset(m_mods_info,0,sizeof(m_mods_info));
	m_mods_num = 0;
	memset(m_mods_list,0,sizeof(m_mods_list));
	m_allmods_is_allowed = false;
	return 0;
}
static int log_mod_is_disable(const char *mod)
{
	uint32_t imod = calc_imod( mod );
	int i = 0; 
	for ( i = 0; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
	{
		if ( m_disable_mods_list[i] == imod && m_disable_mods_list[i] != 0)
			return 1;
	}
	return 0;
}
/**
 * @brief disable log modules
 */
int sw_log_enable_mod(char *mod, int enable)
{
	uint32_t imod = calc_imod( mod );
	int i = 0;
	if ( enable )
	{
		for ( i = 0 ; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
		{
			if ( m_disable_mods_list[i] == imod )
			{
				m_disable_mods_list[i] = 0;
				return 0;
			}
		}
	}
	else
	{
		for ( i = 0 ; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
		{
			if ( m_disable_mods_list[i] == imod )
			{
				return 0;
			}
		}
		for ( i = 0 ; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
		{
			if ( m_disable_mods_list[i] == 0 )
			{
				m_disable_mods_list[i] = imod;
				return 0;
			}
		}			
	}
	return 0;
}

/** 
 * @brief set to output log of all moduls 
 */
int sw_log_add_allmods()
{
	m_allmods_is_allowed = true;
	return 0;
}


/** 
 * @brief return the log modules list.
 *
 * @return the log modules list
 */
char* sw_log_get_mods()
{
	return m_mods_info;
}

/**
 * @brief print current log output info
 */
void sw_log_print_state()
{
	printf("Log state: \n");
	printf("	level: %d\n",m_level);
	printf("	target[%d]:%s\n",m_targets_num,m_targets_info);
	printf("	allow all mods:%d\n",m_allmods_is_allowed);
	printf("	mods[%d]:%s\n",m_mods_num,m_mods_info);
}

/** 
 * @brief output the logging info.
 * 
 * @param level int, the level of the logging info
 * @param mod  char*,the modduls name
 * @param file  char*,the file name
 * @param line  int,the line in the file
 * @param format char*, format string.
 * @param ... 
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log( int level,const char* mod, const char* file, int line, const char *format, ... )
{
	char *logbuf = NULL, *sw_head = NULL;
	int index, size=0, sw_size=0;
	va_list args;
	char local_tm[24] = {0};
	//没有打印目标时返回
	if( 0 == m_targets_num )
		return  -1;

	if( !(m_type==0 || LOG_TYPE_RUN==m_type) )
		return -6;

	if(m_level >= LOG_LEVEL_OFF )		
		return -2;
	
	//判断打印等级
	if( level < m_level || level < 0 || level > LOG_LEVEL_FATAL)
		return -3;
	if (log_mod_is_disable(mod))
		return -5;
	//判断模块是否允许输出日志
	if( !log_mod_is_allowed(mod) )
		return -4;

	//选择生成日志的buf,这里如果日志输出不及时的话就可能日志重复打印了
	index = m_logbuf_index = (m_logbuf_index + 1)&LOG_BUF_INDEX_MASK;
	logbuf = &m_logbuf[index][LOG_HEADER_SIZE];
	sw_head = m_sw_log_header[index];

	//填充日志头信息
	syslog_get_local_time(local_tm,sizeof(local_tm));
	sw_size = snprintf(sw_head,LOG_HEADER_SIZE,"[%u][%s]%s[%s %s %d]%s ", sw_thrd_get_tick(), local_tm, m_level_info[level], mod, file, line, m_pidinfo);

	//填充具体日志信息
	va_start( args, format );
	size = vsnprintf(logbuf,LOG_BUF_SIZE,format, args );
	va_end( args );

	//置字符串结束符
	if( size >= LOG_BUF_SIZE )
		size = LOG_BUF_SIZE - 1;
	if( sw_size >= LOG_HEADER_SIZE )
		sw_size = LOG_HEADER_SIZE -1;
	logbuf[size] = 0;
	sw_head[sw_size] = 0;

	log_output(logbuf,size, sw_head, sw_size);
	return 0;
}

/** 
 * @brief output the logging info.
 * 
 * @param level int, the level of the logging info
 * @param type int, the type of the logging info
 * @param mod  char*,the modduls name
 * @param file  char*,the file name
 * @param line  int,the line in the file
 * @param format char*, format string.
 * @param ... 
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log_syslog( int level, int type, const char* mod, const char* file, int line, const char *format, ... )
{
	char *logbuf = NULL, *sw_head = NULL;
	int index, size=0, sw_size=0;
	va_list args;
	char local_tm[24]={0};
	if(m_level >= LOG_LEVEL_OFF )		
		return -2;
	//没有打印目标时返回
	if( 0 == m_targets_num )
		return  -1;

	if( !(m_type==0 || type==m_type) )
		return -6;
	
	//判断打印等级
	if( level < m_level || level < 0 || level > LOG_LEVEL_FATAL)
		return -3;
	if (log_mod_is_disable(mod))
		return -5;
	//判断模块是否允许输出日志
	if( !log_mod_is_allowed(mod) )
		return -4;

	//选择生成日志的buf
	index = m_logbuf_index = (m_logbuf_index + 1)&LOG_BUF_INDEX_MASK;
	logbuf = &m_logbuf[index][LOG_HEADER_SIZE];
	sw_head = m_sw_log_header[index];

	//填充日志头信息
	syslog_get_local_time(local_tm,sizeof(local_tm));
	sw_size = snprintf(sw_head,LOG_HEADER_SIZE,"[%u][%s]%s[%s %s %d]%s ", sw_thrd_get_tick(), local_tm, m_level_info[level], mod, file, line, m_pidinfo);

	//填充具体日志信息
	va_start( args, format );
	size = vsnprintf(logbuf,LOG_BUF_SIZE,format, args );	//sunniwell log
	va_end( args );

	//置字符串结束符
	if( size >= LOG_BUF_SIZE )
		size = LOG_BUF_SIZE - 1;
	if( sw_size >= LOG_HEADER_SIZE )
		sw_size = LOG_HEADER_SIZE -1;
	logbuf[size] = 0;
	sw_head[sw_size] = 0;

	log_output(logbuf,size, sw_head, sw_size);
	return 0;
}

/**
 *@brief 把日志模块的名字转成数值
 */
static uint32_t calc_imod(const char* mod)
{
	unsigned char t[5];
	memset(t,0,sizeof(t));
	strlcpy((char*)t,mod,sizeof(t));
	return SW_FOURCC(t[0],t[1],t[2],t[3]);
}
/**
 *@brief 查找是否有相同的模块
 */
static int find_imod(uint32_t imod)
{
	int i=0;
	int count = 0;
	for( i =0;i<LOG_MODS_MAX && count<m_mods_num;i++)
	{
		if( m_mods_list[i] != 0 )
			count ++;
		if( m_mods_list[i] == imod)
			return i;
	}
	return -1;
}

/**
 *@brief 判断模块是否允许输出日志
 */
static bool log_mod_is_allowed(const char* mod)
{
	if( m_allmods_is_allowed )
		return true;

	return (find_imod(calc_imod(mod)) >= 0);
}

/**
 *@brief 打开文件
 */
static FILE* log_open_file(char* path,int count)
{	
	char fname[256];
	char suffix[32];
	char index[8];
	char* p = NULL;
	FILE* fp = NULL;

	memset(suffix,0,sizeof(suffix));
    memset(fname, 0, sizeof(fname));
	strlcpy(fname,path,sizeof(fname));
	p = strchr(fname,'.');
	if(p)
	{
		strlcpy(suffix,p,sizeof(suffix));
		*p = '\0';
	}
	snprintf(index,sizeof(index),"%d",count);
    fname[sizeof(fname) - 1] = '\0';
	strlcat(fname,index,sizeof(fname));
	strlcat(fname,suffix,sizeof(fname));
	
	fp = fopen(fname,"w");
	//删除旧的日志文件
	if( count >= 2 )
	{	
        memset(fname, 0, sizeof(fname));
		strlcpy(fname,path,sizeof(fname));
		p = strchr(fname,'.');
		if( p )
			*p = '\0';
		snprintf(index,sizeof(index),"%d",count-2);
        fname[sizeof(fname) - 1] = '\0';
		strlcat(fname,index,sizeof(fname));
		strlcat(fname,suffix,sizeof(fname));
		remove(fname);
	}
	return fp;
}

/**
 *@brief 查找日志输出目标
 */
static int log_find_target(char* target)
{
	int i =0;
	for( i =0;i<LOG_TARGETS_MAX;i++)
	{
		if( m_targets_list[i].type != LOG_TARGET_NULL
			&&  strcmp(target,m_targets_list[i].target_org)==0 )
			return i;
	}
	return -1;

}

/**
 *@brief 找一个空的日志输出目标，以便存储新的日志输出目标
 */
static int log_find_empty_target( )
{
	int i =0;
	for( i =0;i<LOG_TARGETS_MAX;i++)
	{
		if( m_targets_list[i].type == LOG_TARGET_NULL )
			return i;
	}
	return -1;
}


/**
 *@brief 输出日志
 */
static int log_output(char* logbuf,int size, char *sw_head, int sw_size)
{
	int i=0;
	target_t* p_target = NULL;
	FILE* fp = NULL;
	int count  = 0;

	for( i = 0; i<LOG_TARGETS_MAX && count<m_targets_num ; i++ )
	{
		p_target = &(m_targets_list[i]);
		
		if( p_target->type != LOG_TARGET_NULL)
			count++;

		//日志直接输出到控制台
		if( p_target->type == LOG_TARGET_CONSOLE )
		{
			//printf( "%s", logbuf );
			fprintf( stdout, "%s%s", sw_head, logbuf );
		}
#ifndef WIN32
        else if( p_target->type == LOG_TARGET_LOCALSOCK )
        {
            if( p_target->log_union.loglocalsock.sock >= 0 )
            {
                char *p = logbuf- sw_size ;//这里sw_size+1 <= LOG_HEADER_SIZE并且logbuf预留了LOG_HEADER_SIZE的头数据空间
                memcpy(p, sw_head, sw_size);//原来是send(sw_size + 1, size + 1);
                sendto( p_target->log_union.loglocalsock.sock, 
                        p, size  + sw_size + 1, 0, 
                        &(p_target->log_union.loglocalsock.sock_un), 
                        sizeof( p_target->log_union.loglocalsock.sock_un ) );
            }
        }
#endif
		//日志以文件的形式保存于内存或外挂设备
		else if( p_target->type == LOG_TARGET_FILE)
		{
			fp = NULL;
			//确定日志输出文件
			if( p_target->log_union.logfile.fp1_state == LOG_FP_FULL )
			{
				fp = p_target->log_union.logfile.fp1;
			}
			if(  p_target->log_union.logfile.fp2_state == LOG_FP_FULL )
			{
				fp = p_target->log_union.logfile.fp2;
			}

			if( p_target->log_union.logfile.fp1_state == LOG_FP_ON )
			{	
				fp = p_target->log_union.logfile.fp1;
				if( p_target->log_union.logfile.fp1_size >=(p_target->log_union.logfile.maxsize-8192))
				{
					p_target->log_union.logfile.fp1_fullcount = 0;
					p_target->log_union.logfile.fp1_state  = LOG_FP_FULL;
				}
			}
			if( p_target->log_union.logfile.fp2_state == LOG_FP_ON )
			{
				fp = p_target->log_union.logfile.fp2;
				if( p_target->log_union.logfile.fp2_size >=(p_target->log_union.logfile.maxsize-8192) )
				{
					p_target->log_union.logfile.fp2_state = LOG_FP_FULL;
					p_target->log_union.logfile.fp2_fullcount = 0;
				}
			}

			//处理LOG文件打开和关闭
			if( p_target->log_union.logfile.fp1_state == LOG_FP_FULL)
			{
				if( p_target->log_union.logfile.fp2_state == LOG_FP_OFF)
				{	
					p_target->log_union.logfile.fp2_state = LOG_FP_OPEN;
					p_target->log_union.logfile.fp2 = log_open_file(p_target->log_union.logfile.path,p_target->log_union.logfile.count);
					if( p_target->log_union.logfile.fp2 )
					{
						p_target->log_union.logfile.count++;
						if( p_target->log_union.logfile.count >= 4 )
						{	
							p_target->log_union.logfile.count = 0;
						}
						p_target->log_union.logfile.fp2_size = 0;
						p_target->log_union.logfile.fp2_state = LOG_FP_ON;
						fp = p_target->log_union.logfile.fp2;
					}
					else
					{	
						fprintf(stdout, "Open file %s \n failed ...\n",p_target->log_union.logfile.path);		
						p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
					}
				}
				
				p_target->log_union.logfile.fp1_fullcount++;
				if( p_target->log_union.logfile.fp1_fullcount >= 8 
					&& p_target->log_union.logfile.fp1_state==LOG_FP_FULL )
				{
					p_target->log_union.logfile.fp1_state=LOG_FP_CLOSE;	
					if(p_target->log_union.logfile.fp1) 
						fclose(p_target->log_union.logfile.fp1);
					p_target->log_union.logfile.fp1 = NULL;
					p_target->log_union.logfile.fp1_fullcount = 0; 
					p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
				}
			}
			if( p_target->log_union.logfile.fp2_state==LOG_FP_FULL)
			{	
				if( p_target->log_union.logfile.fp1_state == LOG_FP_OFF)
				{
					p_target->log_union.logfile.fp1_state = LOG_FP_OPEN;
					p_target->log_union.logfile.fp1 = log_open_file(p_target->log_union.logfile.path,p_target->log_union.logfile.count);	
					if( p_target->log_union.logfile.fp1)
					{
						p_target->log_union.logfile.count++;
						if( p_target->log_union.logfile.count >= 4 )
						{	
							p_target->log_union.logfile.count = 0;
						}
						p_target->log_union.logfile.fp1_size = 0;
						p_target->log_union.logfile.fp1_state = LOG_FP_ON;
						fp = p_target->log_union.logfile.fp1;
					}
					else
					{
						fprintf(stdout, "Open file %s \n failed ...\n",p_target->log_union.logfile.path);		
						p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
					}
				}
				p_target->log_union.logfile.fp2_fullcount ++;
				if( p_target->log_union.logfile.fp2_fullcount >= 8
					&& p_target->log_union.logfile.fp2_state==LOG_FP_FULL )
				{
					p_target->log_union.logfile.fp2_state=LOG_FP_CLOSE;
					if(p_target->log_union.logfile.fp2)
						fclose(p_target->log_union.logfile.fp2);
					p_target->log_union.logfile.fp2 = NULL;
					p_target->log_union.logfile.fp2_fullcount = 0; 
					p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
				}
			}
				
			//输出日志,到文件
			if(fp)
			{
				//fputs( logbuf, fp);
				fprintf(fp, "%s%s",sw_head, logbuf);
				if( fp  == p_target->log_union.logfile.fp1 )
				{
					p_target->log_union.logfile.fp1_size += sw_size;
					p_target->log_union.logfile.fp1_size += size;
				}
				else if( fp == p_target->log_union.logfile.fp2 )
				{
					p_target->log_union.logfile.fp2_size += sw_size;
					p_target->log_union.logfile.fp2_size += size;
				}
			}
		}
	}
	return 0;
}
