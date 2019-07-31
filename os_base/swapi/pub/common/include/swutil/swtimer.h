/** 
 * @file swtimer.h
 * @brief 定义定时器函数接口,注意在timer的回调函数里只适合处理耗时比较小的事情，
 *		否则会影响整个定时器系统的准确性
 *
 * @author hujinshui/chenkai
 * @date 2005-09-16 created
 */

#ifndef __SW_TIMER_H__
#define __SW_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

//定时器类型
/*typedef enum
{
	TIMER_TYPE_ONESHOT=0,
	TIMER_TYPE_REPEAT
}timer_type_t;
*/

//定时器状态
typedef enum
{
	TIMER_STATUS_DISABLE=0,
	TIMER_STATUS_ENABLE
}timer_status_t;

/**   
 * @brief  线程回调函数,会被反复调用,注意在回调函数里只适合处理时间比较短的事情，
 *		否则会影响整个定时器系统的准确性
 * 
 * @param *ontimehandler_t	指向回调函数的指针
 * @param wparam 			回调函数的第一个参数
 * @param lparam 			回调函数的第二个参数
 * 
 * @return 无
 */
typedef void (*ontimehandler_t)( uint32_t wparam, uint32_t lparam );

/** 
 * @brief 初始化定时器模块
 * 
 * @param priority  线程的优先级 
 *
 * @param stacksize 堆栈的大小
 *
 * @return 0,成功; 错误号, 失败
 */
int sw_timer_init( int priority, int stacksize );

/**
 * @brief 退出timer 模块，释放相应的资源
 * 
 * @return 无
 */
void sw_timer_exit( );

/**
  * @brief 创建一个定时器
  *
  * @param name,定时器的名称
  * @param interval, 定时器时间间隔
  * @param times,定时器重复次数，0表示无限次
  * @param handler,定时器回调函数
  * @param wparam,定时器回调参数1
  * @param lparam,定时器回调参数2
  *
  * @return 句柄 ; NULL,失败
  */
HANDLE sw_timer_create( char* name,int interval,int times,
	ontimehandler_t handler,uint32_t wparam,uint32_t lparam);

/**
  * @brief 销毁定时器
  * @param handle,定时器句柄
  *
  * @return 无
  */
void sw_timer_destroy(HANDLE handle);

/**
  * @brief 重置定时器,状态切换到刚开始创建的状态
  * @param handle,定时器句柄
  *
  * @return 是否重置成功，0 成功，-1失败
  */
int sw_timer_reset(HANDLE handle);

/** 
  * @brief 取得定时器状态
  * @param handle,定时器句柄
  *
  * @return 返回定时器状态
  */
timer_status_t sw_timer_get_status(HANDLE handle);

/** 
 *   * @brief 设置定时器状态
 *     * @param handle,定时器句柄
 *       *
 *         * @return 返回0表示设置成功，返回-1表示设置错误
 *           */
int sw_timer_set_status(HANDLE handle, timer_status_t status);

/**
  * @brief 根据名称取得定时器句柄
  * @param name, timer名称
  *
  *	@return 指定名称的定时器句柄
  */
HANDLE sw_timer_get_byname(char* name);

/**
  * @brief 定时处理函数
  * @param handle,定时器句柄
  * @param handler,定时器回调函数
  * @param wparam,定时器回调参数1
  * @param lparam,定时器回调参数2
  *  
  * @return 是否重置成功，0 成功，-1失败
  */
int sw_timer_set_ontimehandler(HANDLE handle,ontimehandler_t handler,uint32_t wparam,uint32_t lparam);

/**
  * @brief 设置定时器定时时间,该函数会导致定时器重置
  * @param handle,定时器句柄
  * @param interval, 定时器时间间隔  
  * @param times,定时器重复次数，-1表示无限次
  *
  * @return 是否重置成功，0 成功，-1失败
  */
int sw_timer_set_interval(HANDLE handle,int interval,int times);

/**
  * ＠brief 打印所有定时器信息
  * @return 无
  */
void sw_timer_print();

#ifdef __cplusplus
}
#endif

#endif

