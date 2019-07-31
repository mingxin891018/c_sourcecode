/** 
 * @file rand_ext.h
 * @brief 定义随机数生成器
 * @author sunniwell
 * @date 2006-01-05 created
 */

#ifndef __RAND_EXT_H__
#define __RAND_EXT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 产生0～1内的doule型随机数
 * 
 * @return 返回doule型随机数
 */
double drand48();
/** 
 * @brief 产生long型随机数
 * 
 * @return 返回long型随机数
 */
long lrand48();


#ifdef __cplusplus
}
#endif

#endif /* __RAND_EXT_H__ */

