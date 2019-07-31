/** 
 * @file swsignal.h
 * @brief 信号量函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */

#ifndef __SWSIGNAL_H__
#define __SWSIGNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 创建信号量
 * 
 * @param defval 取值为0
 * @param maxval 取值为1
 * 
 * @return 成功,返回信号量句柄; 否则,返回空值
 */
HANDLE sw_signal_create( int defval, int maxval );

/** 
 * @brief 销毁信号量
 * 
 * @param signal 信号量句柄
 */
void sw_signal_destroy( HANDLE signal );

/** 
 * @brief 等待信号量
 * 
 * @param signal 信号量句柄
 * @param timeout -1表示无限等待;其他值表示等待的时间(ms)
 * 
 * @return 成功,返回0; 否则,返回-1
 */
int sw_signal_wait( HANDLE signal, int timeout );

/** 
 * @brief 释放信号量
 * 
 * @param signal 信号量句柄
 */
void sw_signal_give( HANDLE signal );

/** 
 * @brief 重置信号量
 * 
 * @param signal 信号量句柄
 */
void sw_signal_reset( HANDLE signal );

#ifdef __cplusplus
}
#endif

#endif  /* __SWSIGNAL_H__ */
