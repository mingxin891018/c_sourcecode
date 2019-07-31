/************************************************************
* AUTHOR: lijian / huanghuaming / chenkai
* CONTENT: 消息队列函数接口
* NOTE:	
* HISTORY:
* [2005-9-16] created
* [2011-03-09] 增加支持优先级队列，最大支持5个优先级，优先级	
*	的值越小，优先级越高
***********************************************************/
#include "swapi.h"
#include "swutil_priv.h"
#include "swmutex.h"
#include "swqueue.h"
#include "swsignal.h"

#define MAX_PRIORITY_NUM 5  //最大优先级数目

typedef struct
{
	/*优先级等级数目*/
	int level_num;
	/*每个元素的大小*/
	uint32_t esize;
	/* 队列数据 */
	void* elements[MAX_PRIORITY_NUM];
	/*对列总空间大小*/
	uint32_t tsize[MAX_PRIORITY_NUM];
	/* 消息队列中的读位置 */
	void* read[MAX_PRIORITY_NUM];
	/* 消息队列中的写位置 */
	void* write[MAX_PRIORITY_NUM];	
	/*控制消息队列进的信号量*/
	HANDLE write_signal[MAX_PRIORITY_NUM];
	/*是否满了之后覆盖老的元素*/
	bool full_cover[MAX_PRIORITY_NUM];
	/*控制消息队列出的信号量*/
	HANDLE read_signal;
	/* 控制消息队列进和出的信号量*/
	HANDLE mutex;
}sw_queue_t;


/** 
 * @brief 创建队列
 * 
 * @param length 队列长度
 * @param element_size 队列中每个元素的长度
 * @param full_cover 当对列满时，是否允许删除老元素
 *
 * @return 工作句柄
 */
HANDLE sw_queue_create( int length,uint32_t element_size,bool full_cover )
{
	return sw_queue_create_with_priority(element_size,&length,&full_cover,1);
}

/** 
 * @brief 创建多个等级的队列
 * 
 * @param element_size 队列中每个元素的长度
 * @param qsize,每个等级队列的长度
 * @param full_cover 每个等级队列当对列满时，是否允许删除老元素
 * @param level_num  等级的数目,最大等级数目不超过5
 *
 * @return 队列句柄
 */
HANDLE sw_queue_create_with_priority(uint32_t element_size,int* qsize,bool* full_cover,int level_num)
{
	sw_queue_t *q=NULL;
	int i =  0;
	if( qsize == NULL || full_cover==NULL || element_size <=0 || level_num > MAX_PRIORITY_NUM )
		return NULL;

	q= malloc(sizeof(sw_queue_t));
	if( q == NULL )
	{
		UTIL_LOG_FATAL("No enough memory to create a queue!\n");
		return NULL;
	}
	memset(q,0,sizeof(sw_queue_t));
	
	q->esize = element_size;
	q->level_num = level_num;
	for( i=0; i<level_num; i++ )
	{
		q->elements[i] = (void*)malloc(element_size *qsize[i]);
		if( q->elements[i] == NULL )
		{
			UTIL_LOG_FATAL("No enough memory to malloc a queue elements!\n");
			goto ERROR_EXIT;
		}
	
 		q->tsize[i] = element_size *qsize[i];
		q->full_cover[i] = full_cover[i];
		q->read[i]=q->write[i]=q->elements[i];

		q->write_signal[i] = sw_signal_create( 0, 1 );
		if( q->write_signal[i] == NULL )
		{
			UTIL_LOG_FATAL("Fail to ceate a signal!\n");
			goto ERROR_EXIT;
		}
	}

	q->mutex = sw_mutex_create();
	if( q->mutex == NULL )
	{	
		UTIL_LOG_FATAL("Fail to ceate a mutex!\n");
		goto ERROR_EXIT;
	}
	q->read_signal = sw_signal_create( 0, 1 );
	if( q->read_signal == NULL )
	{
		UTIL_LOG_FATAL("Fail to ceate a signal!\n");
		goto ERROR_EXIT;
	}	
	return q;
ERROR_EXIT:
	if(q)
		sw_queue_destroy(q);
	return NULL;

	
}

/** 
 * @brief 释放队列资源
 * 
 * @param handle 工作句柄
 */
void sw_queue_destroy( HANDLE handle )
{
	sw_queue_t *q = (sw_queue_t *)handle;
	int i;

	if(q == NULL )
		return;
	if( q->mutex)
		sw_mutex_lock( q->mutex );	
	for(i=0;i<q->level_num; i++)
	{
		if( q->write_signal[i] )
		{
			sw_signal_destroy(q->write_signal[i]);
			q->write_signal[i] = NULL;
		}

		if(q->elements[i])
		{
			free(q->elements[i]);
			q->elements[i] = NULL;
		}
	}
	q->level_num = 0;
	if( q->mutex)
		sw_mutex_unlock( q->mutex );	

	if( q->read_signal )
	{
		HANDLE rsignal = q->read_signal;
		q->read_signal = NULL;
		sw_signal_destroy(rsignal);
	}

	if( q->mutex)
	{
		HANDLE mutex = q->mutex;
		q->mutex = NULL;
		sw_mutex_destroy(mutex);
	}
	
	free(q);
}

/** 
 * @brief 向队列中增加元素
 * 
 * @param handle 工作句柄
 * @param e 要增加的元素的地址
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post( HANDLE handle,void* e)
{	
	return sw_queue_post_with_level(handle,e,0);
}

/** 
 * @brief 向指定等级的队列中增加元素
 * 
 * @param handle 工作句柄
 * @param e 要增加的元素的地址
 * @param level 优先级等级
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post_with_level(HANDLE handle,void* e,int level)
{
	void* next = NULL;
	bool  is_full = false;
	sw_queue_t *q = (sw_queue_t *)handle;

	if(q == NULL || level >= q->level_num )
		return -1;
	

	sw_signal_wait(q->write_signal[level],0);

	sw_mutex_lock( q->mutex );	
	//计算下一个element位置
	next = q->write[level]<(q->elements[level]+q->tsize[level]-q->esize) ? (q->write[level]+q->esize) : q->elements[level];
	//判断队列是否已经满了
	is_full = (next == q->read[level]);

	sw_mutex_unlock(q->mutex);

	//如果不允许删除老的元素，且下一个element和read指针相等
	//我们需要等待空出一个元素再向队列增加元素
	if( !q->full_cover[level] && is_full)
	{
		sw_signal_wait(q->write_signal[level],WAIT_FOREVER);
	}

	sw_mutex_lock( q->mutex );

	memcpy(q->write[level],e,q->esize);

	q->write[level] = q->write[level]<(q->elements[level]+q->tsize[level]-q->esize) ? (q->write[level]+q->esize) : q->elements[level];
	/* 如果队列满了，删除最老的消息 */
	if(q->full_cover[level] && q->write[level] == q->read[level] )
		q->read[level] =q->read[level]<(q->elements[level]+q->tsize[level]-q->esize) ? q->read[level]+q->esize : q->elements[level];

	sw_mutex_unlock( q->mutex );
	sw_signal_give( q->read_signal);

	return 0;
}

/** 
 * @brief 从队列中提取元素,如果对列为空会一直等待
 * 
 * @param handle 工作句柄
 * @param e 存储提取后元素内容的地址
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read( HANDLE handle,void* e)
{
	return sw_queue_read_timeout(handle,e,-1);
}

/** 
 * @brief 从队列中提取元素
 * 
 * @param handle 工作句柄
 * @param e 存储提取后元素内容的地址
 * @param timeout 提取元素的超时时间，单位ms
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_timeout(HANDLE handle,void* e,int timeout)
{
	int  idx_got = -1;
	bool is_empty = false;
	int i=0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || e==NULL || q->level_num <=0 )
		return -1;	
	
	if( sw_signal_wait(q->read_signal, timeout) != 0 )
		return -1;

	sw_mutex_lock( q->mutex);
	
	for( i = 0;i<q->level_num; i++ )
	{
		if( q->read[i] != q->write[i] &&  idx_got < 0 )
		{
			memcpy(e,q->read[i],q->esize);
			q->read[i] =q->read[i]<(q->elements[i]+q->tsize[i]-q->esize) ? q->read[i]+q->esize : q->elements[i];
			idx_got = i;
		}
		is_empty = (q->read[i] == q->write[i]);
		if( !is_empty )
			break;		
	}
	
	sw_mutex_unlock( q->mutex );

	if( !is_empty )
		sw_signal_give( q->read_signal );

	if( idx_got >=0 )
	{
		sw_signal_give(q->write_signal[idx_got]);
		return 0;
	}
	else
	{
		return -1;
	}
}

/** 
 * @brief 从队列中提取元素，如果对列为空立刻返回
 * 
 * @param handle 工作句柄
 * @param e 存储提取后元素内容的地址
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_nowait( HANDLE handle,void* e)
{
	return sw_queue_read_timeout(handle,e,0);
}

/** 
 * @brief 取得系统消息队列消息数目
 * 
 * @param handle 工作句柄
 * 
 * @return 
 */
int sw_queue_get_num( HANDLE handle )
{
	int i=0;
	int num = 0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <=0 )
		return 0;
	
	sw_mutex_lock( q->mutex);

	for( i=0;i<q->level_num;i++)
	{
		if( q->write[i] > q->read[i] )
		{
			num += (q->write[i]-q->read[i])/q->esize;
		}
		else if( q->write[i] == q->read[i] )
		{
			num += 0;
		}
		else
		{
			num += (q->write[i]+q->tsize[i]-q->read[i])/q->esize;
		}
	}
	sw_mutex_unlock(q->mutex);

	return num;
}

/** 
 * @brief 取得消息队列最大长度
 * 
 * @param handle 工作句柄
 * 
 * @return 
 */
int sw_queue_get_max( HANDLE handle )
{
	int i=0;
	int num = 0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <= 0 )
		return 0;

	for(i=0;i<q->level_num; i++)
		num += q->tsize[i]/q->esize - 1;
	
	return num;
}

/** 
 * @brief 清除消息队列
 * 
 * @param handle 工作句柄
 */
void sw_queue_clear( HANDLE handle )
{
	int i =0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <=0 )
		return;
	
	sw_mutex_lock( q->mutex );
	for( i=0 ;i < q->level_num; i++)
	{
		q->write[i] = q->read[i] = q->elements[i];
	}
	sw_mutex_unlock( q->mutex );

	sw_signal_give(q->write_signal);
}

/** 
 * @brief 清除特定优先级的消息队列
 * 
 * @param handle 工作句柄
 * @param level_num 消息优先级
 */
void sw_queue_clear_with_level( HANDLE handle, int level)
{
	int i =0;
	sw_queue_t *q = (sw_queue_t *)handle;
	if(q == NULL || q->level_num <=0 )
		return;
	
	sw_mutex_lock( q->mutex );
	
	if (0 <= level &&  level < q->level_num )
		q->write[level] = q->read[level] = q->elements[level];
	
	sw_mutex_unlock( q->mutex );

	sw_signal_give(q->write_signal);
}
