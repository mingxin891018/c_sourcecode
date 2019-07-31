/** 
 * @file swthrd.h
 * @brief 线程函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16 created
 */

#ifndef __SWTHRD_H__
#define __SWTHRD_H__


#ifdef __cplusplus
extern "C"
{
#endif
#include "swapi.h"

/**
 * 线程调度策略
 * 
 */
typedef enum
{
	SW_SCHED_NORMAL = 0,		/* 一般策略 */
	SW_SCHED_FIFO,				/*先进先出  */
	SW_SCHED_RR				/* 循环法   */
}sw_policy_t;


/* 推荐线程堆栈大小(字节)*/
#define DEFAULT_STACK_SIZE	64*1024
/* 推荐线程优先级 */
#define DEFAULT_PRIORIOTY	100

/**   
 * @brief  线程回调函数,会被反复调用,返回0表示退出线程,其他表示继续运行 
 * 
 * @param *threadhandler_t	指向回调函数的指针
 * @param wparam 			回调函数的第一个参数
 * @param lparam 			回调函数的第二个参数
 * 
 * @return 真(非0),线程被继续调用; 假(0),线程退出
 */
typedef bool (*threadhandler_t)(uint32_t wparam, uint32_t lparam );

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
				   threadhandler_t handler, uint32_t wparam, uint32_t lparam );
/** 
 * @brief 关闭线程
 * 
 * @param thrd 线程句柄
 * @param ms 等待的时间(单位为毫秒)
 */
void sw_thrd_close( HANDLE thrd, int ms );
/** 
 * @brief 线程是否被打开,  
 * 
 * @param thrd 线程句柄
 * 
 * @return 如果打开返回 真(1); 否则返回假(0) 
 */
unsigned int sw_thrd_is_openned( HANDLE thrd );
/** 
 * @brief 线程是否running,  
 * 
 * @param thrd 线程句柄
 * 
 * @return 如果running返回 真(1); 否则返回假(0) 
 */
bool sw_thrd_is_running( HANDLE thrd );
/** 
 * @brief 设置线程优先级, priority的范围为[0,255],0表示优先级最高,255最低
 * 
 * @param thrd 线程句柄
 * @param priority 线程优先级的大小
 */
void sw_thrd_set_priority( HANDLE thrd, unsigned char priority );
/** 
 * @brief 取得线程优先级,priority的范围为[0,255],0表示优先级最高,255最低
 * 
 * @param thrd 线程句柄 
 * 
 * @return 当前线程的优先级
 */
unsigned char sw_thrd_get_priority( HANDLE thrd );

/** 
 * @brief 线程暂停
 * 
 * @param thrd 线程句柄 
 */
void sw_thrd_pause( HANDLE thrd );
/** 
 * @brief 线程继续
 * 
 * @param thrd 线程句柄 
 */
void sw_thrd_resume( HANDLE thrd );
/** 
 * @brief 线程是否被暂停
 * 
 * @param thrd 线程句柄
 * 
 * @return 如果暂停返回真(true);否则,返回假(false)
 */
bool sw_thrd_is_paused(HANDLE thrd);
	
/** 
 * 设置全局调度策略
 * 
 * @param policy sw_policy_t
 */	
void sw_thrd_set_global_policy(sw_policy_t policy);
	

/** 
 * @brief 线程延迟
 * 
 * @param timeout 超时时间(单位为毫秒) 
 */
void sw_thrd_delay( int timeout );
/** 
 * @brief 取得系统运行时间(ms)
 * 
 * @return 当前的运行时间
 */
unsigned int sw_thrd_get_tick();

/** 
 * @brief 打印所有线程 
 */
void sw_thrd_print();

/** 
 * @brief 获取所有线程信息
 * 
 * @param buf 
 * @param size 
 * 
 * @return 返回填充buf的大小
 */
int sw_thrd_getinfo( char* buf, int size );
	
/** 
 * @brief 按名字查找线程
 * 
 * @param name 名字
 * 
 * @return 线程句柄
 */
HANDLE sw_thrd_find_byname( char* name );
	

#ifdef __cplusplus
}
#endif

#endif  /* __SWTHRD_H__ */
