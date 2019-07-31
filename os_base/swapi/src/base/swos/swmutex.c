/** 
 * @file swmutex.h
 * @brief 互斥量函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */
#include "swapi.h"
#include "swos_priv.h"
#include "swmutex.h"

#define swos_malloc malloc
#define swos_free free

/** 
 * @brief 创建互斥量
 * 
 * @return 成功,返回互斥量句柄; 否则,返回空值
 */
HANDLE sw_mutex_create()
{

	pthread_mutex_t* h = (pthread_mutex_t*)swos_malloc(sizeof(pthread_mutex_t));
	
	if(h)
	{
		if ( pthread_mutex_init(h, NULL) != 0)
		{/* 初始化互斥量失败,错误原因见error */
			perror("pthread_mutex_init: ");
			swos_free(h);
			h = NULL;
		}
	}
	
	return h;
	
}

/** 
 * @brief 销毁互斥量
 * 
 * @param mutex 互斥量句柄
 */
void sw_mutex_destroy( HANDLE mutex )
{
	pthread_mutex_t* h = (pthread_mutex_t*)mutex;

	if(h==NULL)
	{	
		return;
	}
	
	pthread_mutex_destroy(h);
	swos_free(h);
	
}

/** 
 * @brief 加锁
 * 
 * @param mutex 互斥量句柄
 */
void sw_mutex_lock( HANDLE mutex )
{
	if( mutex == NULL )
	{	
		return;
	}
	pthread_mutex_lock((pthread_mutex_t*)mutex );

}

/** 
 * @brief 解锁
 * 
 * @param mutex 互斥量句柄
 */
void sw_mutex_unlock( HANDLE mutex )
{
	if( mutex == NULL )
	{	
		return;
	}
	pthread_mutex_unlock((pthread_mutex_t*)mutex );

}
