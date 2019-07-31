/**
* @file swthrd.c
* @brief 线程函数接口
* @author lijian / huanghuaming / chenkai
* @history
*         2005-09-16 created  
*         2010-07-08 qushihui modified
*/

#include "swapi.h"
#include "swos_priv.h"
#include "swthrd.h"
#include "swmutex.h"

#if (defined(ANDROID) && defined(SUPPORT_SWAPI30))
#include "swjni.h"
#endif

#ifndef WIN32
#include <sys/times.h>
#include <sys/syscall.h>
#endif

#define MAX_THRD_NUM	64           //最大线程数
#define swos_malloc malloc
#define swos_free free

#ifdef ANDROID
#define __sched_priority sched_priority
#endif
#ifdef WIN32
#define __sched_priority sched_priority
#endif

#ifdef ANDROID
#include <sys/prctl.h>
#endif

typedef struct sthrdinfo_t
{
	/* 线程ID */
	pthread_t tid;
	/* 线程属性对象 */
	pthread_attr_t attr;
	/* 父线程PID */
	pid_t ppid;
	/* 线程PID */
	pid_t pid;
	/*线程调度策略*/
	int policy;
	/* 线程的优先级 */
	int priority;
	/* 信号灯 */
	sem_t sem;
	/* 线程回调函数 */
	threadhandler_t handler;
	/* 回调函数参数 */
	unsigned long wparam;
	unsigned long lparam;	
	
	/* 是否暂停 */
	int is_pause;
	/* 表示当前线程是否退出 */
	int is_exit;
	/* 任务名称 */
	char name[32];
}sthrdinfo_t;

static sthrdinfo_t *m_all[MAX_THRD_NUM];			//线程信息
static int m_ref = -1;								//线程数
static HANDLE m_mutex = NULL; 						//互斥锁
static int m_global_policy = -1;					//调度策略

static void* sema_thread_handler( void* lp_param );	//线程回调函数

/** 
 * @brief 创建线程,打开后处于暂停状态,priority=0(最高),priority=255(最低)
 * 
 * @param name 		创建的线程名
 * @param priority 	线程优先级
 * @param policy 	调度策略 sw_sched_t
 * @param stack_size 堆栈的大小
 * @param handler 	指向回调函数的指针
 * @param wparam 	回调函数的第一个参数
 * @param lparam 	回调函数的第二个参数
 * 
 * @return 成功,返回线程句柄;失败,返回空值
 */
HANDLE sw_thrd_open( char *name, unsigned char priority, sw_policy_t policy, int stack_size, 
					threadhandler_t handler, uint32_t wparam, uint32_t lparam )
{
	int i, rc;
	pthread_attr_t *pattr = NULL;
	sthrdinfo_t *info = NULL;
	struct sched_param param;
	int max_priority;
	int min_priority;

	if( m_ref < 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		m_ref = 0;
		m_mutex = sw_mutex_create();
	}

	if( m_mutex )
	{
		sw_mutex_lock( m_mutex );
	}
	/* 检测自己退出的线程, 释放资源 */
	if( m_ref > MAX_THRD_NUM - 2 )
	{	
		for( i=0; i < MAX_THRD_NUM; i++ )
		{
#ifdef WIN32
            //ATTENTION m_all[i]->tid != 0在mingw下编译不过, mingw下pthread_t定义为结构体
			if( m_all[i] && *((int *)&m_all[i]->tid) != 0 && m_all[i]->handler == NULL )
#else
			if( m_all[i] &&  m_all[i]->tid  != 0 && m_all[i]->handler == NULL )
#endif
			{
				OS_LOG_DEBUG( "Release self-exit thread<%s:%x>\n", m_all[i]->name, (unsigned int)&m_all[i] );			
				pthread_attr_destroy( &m_all[i]->attr );
				sem_destroy( &m_all[i]->sem );
				memset( m_all[i], 0, sizeof(sthrdinfo_t) );
				swos_free( m_all[i] );
				m_all[i] = NULL;
				m_ref--;
			}
		}
	}
	
	if( m_ref >= MAX_THRD_NUM )
	{
		OS_LOG_ERROR( "Thread num reach max(%d)!!\n", MAX_THRD_NUM );
		sw_mutex_unlock( m_mutex );

		return NULL;
	}
	info = (sthrdinfo_t*)swos_malloc( sizeof(sthrdinfo_t) );
	
	if( info == NULL )
	{
		OS_LOG_DEBUG( "error: sw_thrd_open() failed\n" );
		if( m_mutex )
		{
			sw_mutex_unlock( m_mutex );
		}
		
		return NULL;
	}
	
	memset( info, 0, sizeof(sthrdinfo_t) );
	info->handler = handler;
	info->wparam = wparam;
	info->lparam = lparam;
	info->policy = policy;
	info->priority = (int)priority;
	info->is_pause = 1;

	if( m_global_policy != -1 )
	{
		info->policy = m_global_policy;
	}
	
	strlcpy( info->name, name, sizeof(info->name) );
	
	if( info->policy < SW_SCHED_NORMAL || info->policy > SW_SCHED_RR )
	{
		info->policy = SW_SCHED_NORMAL;
	}
	/* 初始化线程属性对象 */
	pattr = &( info->attr );
	pthread_attr_init( pattr );

	//设置线程堆栈大小
	if( stack_size < 65536 )
	{
		stack_size = 65536;
	}

	rc = pthread_attr_setstacksize( pattr, stack_size );
	if( rc != 0 )
	{
		unsigned int defsize = 0;

		pthread_attr_getstacksize( pattr, &defsize );
		OS_LOG_ERROR( "sw_thrd_open [%s]: set stack size failed! %d, using default size %d\n", name, stack_size, defsize );
	}
	
	pthread_attr_setdetachstate( pattr, PTHREAD_CREATE_DETACHED );
	
	/*设置调度策略*/
	pthread_attr_setschedpolicy( pattr, info->policy /* SCHED_RR */ );

	/*在linux系统上, 线程优先级取值范围为1~99, 1最低, 99最高*/
	param.__sched_priority = 1;
	max_priority = sched_get_priority_max( info->policy );
	min_priority = sched_get_priority_min( info->policy );

	priority = max_priority - ( priority * ( max_priority - min_priority ) ) / 255;
	param.__sched_priority = priority;

	/* 创建信号灯 */
	if( sem_init( &( info->sem ), 0, 0 ) == -1 )
	{
		if( pattr )
		{
			pthread_attr_destroy( pattr );
		}
		goto ERROR_EXIT;
	}

	/* 开始运行线程 */
	if( pthread_create( &(info->tid), pattr, sema_thread_handler, (void*)info ) != 0 )
	{
		if( pattr )
		{
			pthread_attr_destroy( pattr );
		}
		sem_destroy( &(info->sem) );
		goto ERROR_EXIT;
	}

	//线程运行时设置优先级
	pthread_setschedparam( info->tid, info->policy, &param );

	/* 保存线程句柄 */
	for( i=0; i < MAX_THRD_NUM; i++ )
	{
		if( m_all[i] == NULL )
		{
			m_all[i] = info;
			m_ref++;
			break;
		}
	}
	OS_LOG_DEBUG( "Thread-<%s : %d : %x> opened\n", info->name,i,(unsigned int)info);

	if( m_mutex )
	{
		sw_mutex_unlock( m_mutex );
	}

	return info;

ERROR_EXIT:
	info->handler = NULL;
    memset(&info->tid, 0, sizeof(info->tid));
	if( m_mutex )
	{
		sw_mutex_unlock( m_mutex );
	}
	OS_LOG_DEBUG( "error: sw_thrd_open() failed !\n");
	
	return NULL;
}

/** 
 * @brief 关闭线程
 * 
 * @param thrd 线程句柄
 * @param ms 等待的时间(单位为毫秒)
 */
void sw_thrd_close( HANDLE thrd, int ms )
{
	int i = 0;
	int status = 0;
	void *result;
	sthrdinfo_t *info = (sthrdinfo_t *)thrd;

	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!\n" );
		return;
	}

	if( info == NULL )
	{
		return;
	}

	info->is_exit = 1;
	sem_post( &(info->sem) );

	OS_LOG_DEBUG( "Thread-<%s : %d : %x> closing [%d]...\n", info->name,i,(unsigned int)info, ms );

	/* 等待线程自己退出 */
	while( info->handler && ms > 0 )
	{
		usleep( 10*1000 );
		ms -= 10;
	}

	if( m_mutex )
	{
		sw_mutex_lock( m_mutex );
	}

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( info == m_all[i] )
		{
			break;
		}
	}

	if( MAX_THRD_NUM <= i )
	{
		if( m_mutex )
		{
			sw_mutex_unlock( m_mutex );	
		}

		return;
	}

	if( info->handler )
	{
#ifdef ANDROID
		/* 强制退出 */
		if( !pthread_kill(info->tid,SIGUSR1) )
		{
			OS_LOG_DEBUG( "Thread<%s:%x> is kill!!!\n", info->name, (unsigned int)info );
			usleep( 10*1000 );
		}
#else
		/* 强制退出 */
		if( !pthread_cancel(info->tid) )
		{
			OS_LOG_DEBUG( "Thread<%s:%x> is canneled!!!\n", info->name, (unsigned int)info );
		}
		else
		{
			status = pthread_join( info->tid, &result );
			if( status == 0 )
			{
				if (result == PTHREAD_CANCELED )
				{
					OS_LOG_DEBUG( "Thread<%s:%x> canceled!!!\n", info->name, (unsigned int)info);//如果线程是被取消的，回打印。
				}
				else
				{
					OS_LOG_DEBUG( "result:%d\n", (int)result);
				}
			}
		}
#endif
	}
	
	pthread_attr_destroy( &info->attr );
	sem_destroy( &info->sem );
	memset( info, 0, sizeof(*info) );
	m_all[i] = NULL;
	swos_free( info );
	
	m_ref--;

	if( m_mutex )
	{
		sw_mutex_unlock( m_mutex );
	}

	return;
}

/** 
 * @brief 线程是否被打开,  
 * 
 * @param thrd 线程句柄
 * 
 * @return 如果打开返回 真(1); 否则返回假(0) 
 */
unsigned int sw_thrd_is_openned( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return false;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;

	if( info == NULL )
	{
		return false;
	}

#ifdef WIN32
	return ( *((int *)&info->tid) != 0 );
#else
	return ( info->tid != 0 );
#endif
}

/** 
 * @brief 线程是否running
 * 
 * @param thrd 线程句柄
 * 
 * @return 如果running返回真(true);否则,返回假(false)
 */
bool sw_thrd_is_running( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return false;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if(info->handler == NULL)	
		return false;
	if( info->is_pause == 1 )
		return false;
	return true;
}

/** 
 * @brief 设置线程优先级, priority的范围为[0,255],0表示优先级最高,255最低
 * 
 * @param thrd 线程句柄
 * @param priority 线程优先级的大小
 */
void sw_thrd_set_priority( HANDLE thrd, unsigned char priority )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return;
	}

	struct sched_param param;
	sthrdinfo_t *info = (sthrdinfo_t *)thrd;

	if( info == NULL )
	{
		return;
	}
	info->priority = (int)priority;
	priority = sched_get_priority_max( info->policy ) - 
					( priority * ( sched_get_priority_max( info->policy ) - sched_get_priority_min( info->policy ) ) ) / 255;
	param.__sched_priority = priority;
	pthread_setschedparam( info->tid, SCHED_RR, &param );

	return;
}

/** 
 * @brief 取得线程优先级,priority的范围为[0,255],0表示优先级最高,255最低
 * 
 * @param thrd 线程句柄 
 * 
 * @return 当前线程的优先级
 */
unsigned char sw_thrd_get_priority( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return 0;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	return info->priority;	
}

/** 
 * @brief 线程暂停
 * 
 * @param thrd 线程句柄 
 */
void sw_thrd_pause( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if( info )
	{
		info->is_pause = 1;
	}

	return;
}

/** 
 * @brief 线程是否被暂停
 * 
 * @param thrd 线程句柄
 * 
 * @return 如果暂停返回真(true);否则,返回假(false)
 */
bool sw_thrd_is_paused( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return false;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if( info )
	{
		return info->is_pause;
	}
	
	return false;	
}

/** 
 * @brief 线程继续
 * 
 * @param thrd 线程句柄 
 */
void sw_thrd_resume( HANDLE thrd )
{
	if( !thrd )
	{
		OS_LOG_DEBUG( "error!thrd is NULL!" );
		return;
	}

	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	
	if( info )
	{
		info->is_pause = 0;
		sem_post( &(info->sem) );
	}

	return;
}

/** 
 * @brief 线程延迟
 * 
 * @param timeout 超时时间(单位为毫秒) 
 */
void sw_thrd_delay( int timeout )
{
	usleep( timeout*1000 );

	return;
}

/** 
 * @brief 取得系统运行时间(ms)
 * 
 * @return 当前的运行时间
 */
unsigned int sw_thrd_get_tick()
{
#ifdef WIN32
    return timeGetTime();
#else
	static uint32_t timeorigin;
	static bool firsttimehere = true;

	uint32_t now = times(NULL);
	
	//fix hi3560 platform bug
	if(now == (uint32_t)-1)
	{
		now = -errno;
	}

	if( firsttimehere )
	{
		timeorigin = now;
		firsttimehere = false;
	}

	return ( now - timeorigin )*10;
#endif
}


#ifdef ANDROID

/** 
 * @brief 按线程号查找线程
 * 
 * @param name 名字
 * 
 * @return 线程句柄
 */
static HANDLE sw_thrd_find_pidname(pid_t pid)
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler && pid == m_all[i]->pid )
		{
			return m_all[i];
		}
	}

	return NULL;
}


static void thread_exit_handler(int sig)  
{   
	HANDLE thrd = sw_thrd_find_pidname(getpid());
	sthrdinfo_t *info = (sthrdinfo_t *)thrd;
	if (info)
		OS_LOG_DEBUG( "On Live:Thread<%s:%x> is exit!!!\n", info->name, (unsigned int)info );
	pthread_exit(0);  
}  
#endif
/** 
 * @brief 线程开始
 * 
 * @param lp_param 
 */
static void* sema_thread_handler( void* lp_param )
{
#ifdef ANDROID
	struct sigaction actions;  
	memset(&actions, 0, sizeof(actions));   
	sigemptyset(&actions.sa_mask);  
	actions.sa_flags = 0;   
	actions.sa_handler = thread_exit_handler;  
	if(sigaction(SIGUSR1,&actions,NULL)<0)
	{
		OS_LOG_DEBUG("sigaction failed !\n");
	}  
#endif
	sthrdinfo_t *info = (sthrdinfo_t *)lp_param;
	pid_t tid = 0;

#if defined(WIN32)
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL); //允许退出线程
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL ); //设置立即取消
#elif defined(ANDROID)
	info->ppid = getpid();
	info->pid = gettid();
	tid = info->pid;
	int err = prctl(PR_SET_NAME, info->name); //把参数arg2作为调用进程的经常名字, Since Linux 2.6.11
	if(err != 0)
		OS_LOG_ERROR("prctl thread name fail\n");
	OS_LOG_DEBUG( "Thread-<%p, pid:%d, tid:%d, name:%s> handler\n",info, info->pid, tid, info->name);
#ifdef SUPPORT_SWAPI30
	sw_jni_get_env();
#endif
#else
	info->ppid = getppid();
	info->pid = syscall( SYS_gettid );/*getpid()*/;
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL); //允许退出线程
	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL ); //设置立即取消
#endif

	while( !info->is_exit )
	{
		
		if( info->is_pause )
		{
			sem_wait( &(info->sem) );
		}
		/* 增加测试点 */
#ifndef ANDROID
		pthread_testcancel();
#endif

		if( info->handler == NULL || info->is_exit )
		{
			break;
		}

		if( !info->handler( info->wparam, info->lparam ) )
		{
			break;
		}
	}

	info->handler = NULL;

#if (defined(ANDROID) && defined(SUPPORT_SWAPI30))
	sw_jni_del_env();
#endif

	OS_LOG_DEBUG( "Thread-<%s:%x,pid:%d,tid:%d> exit self\n", info->name, (unsigned int)info, info->pid, tid );

	return NULL;
}

/** 
 * @brief 设置全局调度策略
 * 
 * @param policy sw_policy_t
 */	
void sw_thrd_set_global_policy( sw_policy_t policy )
{
	m_global_policy = policy;
}

/** 
 * @brief 打印所有线程 
 */
void sw_thrd_print()
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if(m_all[i] && m_all[i]->handler )
		{
			OS_LOG_INFO( "%-2d 0x%08x %-14s ppid=%d pid=%d proc=0x%-8x policy=%d priority=%d pause=%d \n", 
				   			i, (int)(m_all[i]), m_all[i]->name,m_all[i]->ppid, m_all[i]->pid, (int)m_all[i]->handler, 
				   			m_all[i]->policy, m_all[i]->priority, m_all[i]->is_pause);
		}
	}
	OS_LOG_INFO( "\nref = %d\n", m_ref );
	
	return;
}

/** 
 * @brief 获取所有线程信息
 * 
 * @param buf 
 * @param size 
 * 
 * @return 返回填充buf的大小
 */
int sw_thrd_getinfo( char* buf, int size )
{
	if (buf == NULL || size <= 0)
		return 0;
	char sz_buf[512];
	int i, n, len;

	*buf = 0;
	len = 0;
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler )
		{
			n = snprintf( sz_buf, sizeof(sz_buf), "%-2d 0x%08x %-14s ppid=%d pid=%d proc=0x%-8x policy=%d priority=%-3d pause=%d \n", 
				   i, (int)(m_all[i]), m_all[i]->name,m_all[i]->ppid, m_all[i]->pid,(int)m_all[i]->handler,
				   m_all[i]->policy, m_all[i]->priority, m_all[i]->is_pause);
			if( n < (int)sizeof(sz_buf) && len + n < size )
			{
				strlcpy( buf+len, sz_buf, len < size ? size-len : 0);
				len += n;
			}
		}
	}
	n = snprintf( sz_buf, sizeof(sz_buf), "\nref = %d\n", m_ref );
	if( len + n < size )
	{
		strlcpy( buf+len, sz_buf, len < size ? size-len : 0);
	}

	return len;
}

/** 
 * @brief 按名字查找线程
 * 
 * @param name 名字
 * 
 * @return 线程句柄
 */
HANDLE sw_thrd_find_byname( char* name )
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler && !strcmp( name, m_all[i]->name ) )
		{
			return m_all[i];
		}
	}

	return NULL;
}

/** 
 * @brief 按线程号查找线程名,这个返回的字符串非线程安全,单线程立即使用
 * 
 * @param tid 线程号
 * 
 * @return 线程名
 */
const char *sw_thrd_getname_bypid(pid_t tid)
{
	static char name[32];
	int i;
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] && m_all[i]->handler && m_all[i]->pid == tid)
		{
			strlcpy(name, m_all[i]->name, sizeof(name));
			return &name[0];//返回static是为防止使用m_all[i]->name时线程退出导致内存释放引起的死机
		}
	}
	return "";
}
