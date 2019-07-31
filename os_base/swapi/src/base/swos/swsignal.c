/** 
 * @file swsignal.h
 * @brief 信号量函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */
#include "swapi.h"
#include "swos_priv.h"
#include "swsignal.h"

typedef struct sw_signal_t
{
  sem_t sem;
  int max_val;
}sw_signal_t;

#define swos_malloc malloc
#define swos_free free

/** 
 * @brief 创建信号量
 * 
 * @param defval 取值为0
 * @param maxval 取值为1
 * 
 * @return 成功,返回信号量句柄; 否则,返回空值
 */
HANDLE  sw_signal_create( int defval, int maxval )
{
	sw_signal_t* h = (sw_signal_t*)swos_malloc( sizeof( sw_signal_t ) );

	if( h == NULL )
	{	
		return NULL;
	}
	if ( sem_init( &(h->sem), 0, defval ) == -1)
	{/* 初始化信号量失败,失败原因见error */
		perror("sem_init: ");
		swos_free( h );
		h = NULL;
	}
	else
		h->max_val = maxval;
	return h;
}

/** 
 * @brief 销毁信号量
 * 
 * @param signal 信号量句柄
 */
void sw_signal_destroy( HANDLE sem )
{
	if( sem == NULL )
	{
		return;
	}
	
	sem_destroy( (sem_t*)sem );
	swos_free( sem );
}

/** 
 * @brief 等待信号量
 * 
 * @param signal 信号量句柄
 * @param timeout -1表示无限等待;其他值表示等待的时间(ms)
 * 
 * @return 成功,返回0; 否则,返回-1
 */
int sw_signal_wait( HANDLE sem, int timeout )
{
	struct timespec tv;
     	
	if( sem == NULL )
	{	
		return 0;
	}
	
	if( timeout < 0 )
	{/* 需要判断返回值,因为如果信号量被destroy了也会返回的 */
		return sem_wait( (sem_t*)sem ) == 0 ? 0 : -1;
	}
	else
	{
#ifdef WIN32
        gettimeofday(&tv, NULL);
#else
	 	clock_gettime( CLOCK_REALTIME, &tv );
#endif

		tv.tv_nsec += (timeout%1000)*1000000; 
		if( tv.tv_nsec>= 1000000000 )
		{
			tv.tv_sec +=1;
			tv.tv_nsec-=1000000000;
		}
		tv.tv_sec += timeout/1000;
		if( sem_timedwait( (sem_t*)sem, (const struct timespec *)&tv ) )
		{
			return -1;
		}
		return 0;
	}
}

/** 
 * @brief 释放信号量
 * 
 * @param signal 信号量句柄
 */
void sw_signal_give( HANDLE sem )
{
	int val;

	if( sem == NULL )
	{
		return;
	}

	sem_getvalue( (sem_t*)sem, &val );

	if( val < ((sw_signal_t*)sem)->max_val )
	{
		sem_post( (sem_t*)sem );
	}
}

/** 
 * @brief 重置信号量
 * 
 * @param signal 信号量句柄
 */
void sw_signal_reset( HANDLE sem )
{
	sw_signal_give( sem );
}
