/** 
 * @file swmutex.h
 * @brief 互斥量函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */

#ifndef __SWMUTEX_H__
#define __SWMUTEX_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 创建互斥量
 * 
 * @return 成功,返回互斥量句柄; 否则,返回空值
 */
HANDLE sw_mutex_create();

/** 
 * @brief 销毁互斥量
 * 
 * @param mutex 互斥量句柄
 */
void sw_mutex_destroy( HANDLE mutex );

/** 
 * @brief 加锁
 * 
 * @param mutex 互斥量句柄
 */
void sw_mutex_lock( HANDLE mutex );

/** 
 * @brief 解锁
 * 
 * @param mutex 互斥量句柄
 */
void sw_mutex_unlock( HANDLE mutex );

#ifdef __cplusplus
}
#endif

#endif  /* __SWMUTEX_H__  */
