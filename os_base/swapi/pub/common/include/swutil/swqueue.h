/** 
 * @file swqueue.h
 * @brief 队列函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-9-16 created
 * 		 2011-03-09 实现多个等级的队列
 */


#ifndef __SWQUEUE_H__
#define __SWQUEUE_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief 创建队列
 * 
 * @param length 队列长度
 * @param element_size 队列中每个元素的长度
 * @param full_cover 当对列满时，是否允许删除老元素
 *
 * @return 工作句柄
 */
HANDLE sw_queue_create(int length,uint32_t element_size,bool full_cover );

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
HANDLE sw_queue_create_with_priority(uint32_t element_size,int* qsize,bool* full_cover,int level_num);


/** 
 * @brief 释放队列资源
 * 
 * @param handle 工作句柄
 */
void sw_queue_destroy( HANDLE handle );

/** 
 * @brief 向队列中增加元素
 * 
 * @param handle 工作句柄
 * @param e 要增加的元素的地址
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post( HANDLE handle, void* e);

/** 
 * @brief 向指定等级的队列中增加元素
 * 
 * @param handle 工作句柄
 * @param e 要增加的元素的地址
 * @param level 优先级等级
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_post_with_level(HANDLE handle,void* e,int level);


/** 
 * @brief 从队列中提取元素,如果对列为空会一直等待
 * 
 * @param handle 工作句柄
 * @param e 存储提取后元素内容的地址
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read( HANDLE handle,void* e);

/** 
 * @brief 从队列中提取元素
 * 
 * @param handle 工作句柄
 * @param e 存储提取后元素内容的地址
 * @param timeout 提取元素的超时时间，单位ms
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_timeout( HANDLE handle,void* e,int timeout);

/** 
 * @brief 从队列中提取元素，如果对列为空立刻返回
 * 
 * @param handle 工作句柄
 * @param e 存储提取后元素内容的地址
 * 
 * @return 0=sucess,-1=failed
 */
int sw_queue_read_nowait( HANDLE handle,void* e);


/** 
 * @brief 清除消息队列
 * 
 * @param handle 工作句柄
 */
void sw_queue_clear( HANDLE handle );

/** 
 * @brief 取得系统消息队列消息数目
 * 
 * @param handle 工作句柄
 * 
 * @return 
 */
int sw_queue_get_num( HANDLE handle );

/** 
 * @brief 取得消息队列最大长度
 * 
 * @param handle 工作句柄
 * 
 * @return 
 */
int sw_queue_get_max( HANDLE handle );


#ifdef __cplusplus
}
#endif

#endif /* __SWQUEUE_H__ */
