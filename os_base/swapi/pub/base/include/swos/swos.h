/**                                                              
 * @file swos.h
 * @brief swos 模块初始化
 * @author qushihui
 * @date 2010-07-09
 */

#ifndef __SWOS_H__
#define __SWOS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 添加配置库
 * 
 */
void sw_os_init();


/**
 * @brief 释放资源
 */
void sw_os_exit();

#ifdef __cplusplus
}
#endif

#endif //__SWOS_H__

