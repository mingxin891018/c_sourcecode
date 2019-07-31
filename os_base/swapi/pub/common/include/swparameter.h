/**
 * @file swparameter.h
 * @brief 定义参数管理接口
 * @author Dou Hongchen / huanghuaming
 * @history 2007-02-14 created
 *			 2011-02-28 chenkai add idepot 处理
 */

#ifndef __SW_PARAMETER_H__
#define __SW_PARAMETER_H__

#include "swidepot.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*定义参数观察者*/
typedef void (*on_para_modified)(char* name,char* value,void* handle);

/** 
 * @brief 初始化参数配置
 *	
 * @param max_buf_size 参数模块缓冲区的大小，推荐值是128*1024
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_init(int max_buf_size);

/** 
 * @brief 是否已经初始化
 * 
 * @return true,初始化; false, 未初始化
 */
bool sw_parameter_is_init();

/** 
 * @brief 释放资源
 */
void sw_parameter_exit();

/**
 *	@brief 注册参数仓库,注册之后参数仓库里的函数通过遍历接口加载到parameter里
 */
bool sw_parameter_register_depot( sw_idepot_t* depot,bool isdefault);

/**
 * @breif 根据名称获取参数仓库
 */
sw_idepot_t* sw_parameter_get_depot(char* name);


/**
 * @brief 卸载参数仓库，删掉paraemeter里存在该仓库里的参数
 */
bool sw_parameter_unregister_depot( char* name );

/*
 * @brief 将指定的depot数据分发到swparameter中
 * @param sw_paradepot_t* p_depot, 更新所需要的depot
 * @return 成功返回true,失败返回false
 */
bool sw_parameter_updatefrom_depot( sw_idepot_t* depot );

/** 
 * @brief 保存机顶盒参数, 保存目标由load方式决定
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_save();


/**
 * @brief 设置参数值改变的回调函数
 */
void sw_parameter_set_observer(on_para_modified on_modified,void* handle);

/** 
 * @brief 读取文本区域的参数
 * 
 * @param name 
 * @param value 
 * @param size 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_get( char* name, char* value, int size );

/** 
 * @brief 设置文本区域的参数
 * 
 * @param name 
 * @param value 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set( char* name, char* value );

/** 
 * @brief 设置一组参数
 * 
 * @param buf 
 * @param size 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set_group( char *buf, int size );

/** 
 * @brief 读取文本区域的参数,返回参数的数值
 * 
 * @param name 
 * 
 * @return 参数的数值
 */
int sw_parameter_get_int( char* name );

/** 
 * @brief 按数值设置文本区域的参数
 * 
 * @param name 
 * @param value 参数的数值
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set_int( char* name, int value );


/** 
 * @brief 设置参数的缺省值,当参数不存在则加到参数表中，如果参数存在，且参数仓库没有改变则不做任何处理
 *	如果参数仓库级别更高，则更改参数仓库	
 * 
 * @param name 参数名称
 * @param value 参数值
 * @param depot 指定的参数仓库
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set_default( char* name, char* value,sw_idepot_t* depot);

/**
 * @brief 从内存中添加一组默认的参数到默认的参数仓库中，如果参数存在则保留员参数,参数不存在则添加到默认的仓库中
 *
 * @param buf,参数数组
 * @param size,数组大小
 *
 * return true,更新成功; false,更新失败
 */
bool sw_parameter_set_group_default(char *buf, int size);

/** 
 * @brief 增加参数到指定的参数仓库
 * 
 * @param name 参数名称
 * @param value 参数值
 * @param depotname 指定的参数仓库
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_add( char* name, char* value,char* depotname );

/** 
 * @brief 删除文本区域的参数
 * 
 * @param name 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_delete( char* name );

/** 
 * @brief 清除所有参数
 */
void sw_parameter_delete_all();

/** 
 * @brief 取得第一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_first( unsigned long *pos );

/** 
 * @brief 取得下一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_next( unsigned long *pos );

/** 
 * @brief 取得上一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_prev( unsigned long *pos );

/** 
 * @brief 取得最后一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_last( unsigned long *pos );

/** 
 * @brief 取得参数的POS参数,返回NULL表示没有找到
 * 
 * @param name 
 * @param pos 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_get_pos( char *name, unsigned long *pos );

/** 
 * @brief 根据POS读取参数,返回NULL表示没有找到
 * 
 * @param pos 
 * @param *value 
 * 
 * @return 
 */
char* sw_parameter_get_by_pos( unsigned long pos, char **value );

/** 
 * @brief 取得参数个数
 * 
 * @return 
 */
int sw_parameter_get_num();

/**
 * @brief 设置某个参数只读
 *
 * @param name 参数名称
 * @param readonly 是否只读 
 *
 * return 成功返回true，失败返回false
 */
bool sw_parameter_set_readonly(char* name,bool readonly);


/**
 * @brief 取得某个参数的只读状态
 *
 * @param name 参数名称
 *
 * return 返回只读状态
 */
bool sw_parameter_get_readonly(char* name);

/**
 * @brief 设置参数是否实时读取:目前只对java有用,默认的java数据库参数都是实时读取的
 *
 * @param name 要实时读写的参数名程
 *
 * return 成功返回true，失败返回false
 */
bool sw_parameter_set_realtime(char* name,bool realtime);

/**
 * @brief 强制修改对应的参数仓库位置,并设置其只读属性,java参数的实时读取属性,如果是java参数的话需要从APK数据库更新参数值,
 					如果参数库位置更改,原始有值的话不修改参数值，否则设为默认值,
 					此接口只允许在app初始化时调用
 *
 * @param name,参数名
 * @param defaultvalue,参数默认值
 * @param readonly,参数只读
 * @param realtime,实时读取
 * @param depot,实时读取
 *
 * return true,参数库位置改变后返回true
 */
bool sw_parameter_update_with_depot(char *name, char *defaultvalue, bool readonly, bool realtime, sw_idepot_t* depot);

/**
 * @brief 刷新java参数库中的只读和非实时读取的参数
 */
void sw_parameter_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /*__SW_PARAMETER_H__*/
