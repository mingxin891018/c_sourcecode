/** 
 * @file swproductimg.h
 * @brief make product image interface
 * Copyright (c) 2009 Beijing Sunniwell Broadband Digital Technologies, Inc. All  Rights Reserved.
 * @author sunke [2009-12-21] created from v1.0 by sunke
 * @date 2009-12-21
 */
#ifndef __SW_PRODUCT_IMG_H__
#define __SW_PRODUCT_IMG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_PARTITION_NUM 20
#define MAX_BUFFER_LEN (256*1024)
#define PRODUCT_IMG_BASE_HEAD_LEN (16*5)

/**
 * 升级状态类型
 * 
 */
typedef enum
{
	SWUPGPRODIMG_STATUS_BEGIN_UPGRADE = 1,       /**< 开始升级 */

	SWUPGPRODIMG_STATUS_BEGIN_DOWNLOAD, /**< 开始下载 */
	SWUPGPRODIMG_STATUS_DO_DOWNLOAD,		/**< 正在下载， value：下载百分比 */
	SWUPGPRODIMG_STATUS_END_DOWNLOAD,	/**< 下载结束 */

	SWUPGPRODIMG_STATUS_BEGIN_DECRYPTION, /**< 开始解密 */
	SWUPGPRODIMG_STATUS_DO_DECRYPTION,		/**< 正在解密， value：下载百分比 */
	SWUPGPRODIMG_STATUS_END_DECRYPTION,	/**< 解密结束 */

	SWUPGPRODIMG_STATUS_BEGIN_ERASE,	    /**< 开始擦出， value：说明升级包类型 */
	SWUPGPRODIMG_STATUS_DO_ERASE,		/**< 正在擦出， value：擦出百分比  */
	SWUPGPRODIMG_STATUS_END_ERASE,       /**< 擦除结束 */

	SWUPGPRODIMG_STATUS_BEGIN_WRITE,	    /**< 开始升级(写flash) ， value：说明升级包类型 */
	SWUPGPRODIMG_STATUS_DO_WRITE,		/**< 正在升级， value：写入百分比  */
	SWUPGPRODIMG_STATUS_END_WRITE,       /**< 写入结束 */
	SWUPGPRODIMG_STATUS_END_UPGRADE,       /**< 升级结束 */
	SWUPGPRODIMG_STATUS_ERROR,           /**< 出错 value：出错状态码*/
	SWUPGPRODIMG_STATUS_INIT,
	SWUPGPRODIMG_STATUS_EXIT,

}swupgprodimg_status_t;

typedef int (*PUpgradeProdimgStatusCallback)( int status, int value,  unsigned long param);

bool sw_upgrade_productimage_init();
void sw_upgrade_productimage_exit();
int sw_upgrade_productimage(const char *infilename);
bool sw_upgrade_prodimg_is_busy();
void sw_upgrade_productimg_set_status_callback(PUpgradeProdimgStatusCallback pProc, uint32_t param );
#if 0
int sw_print_allblksstat();
bool sw_print_partition();
bool sw_erase_partition( const char *mtd_device );
bool sw_reset_partitionoob( const char *mtd_device );
bool sw_reset_addroob( uint32_t startaddr,uint32_t endaddr );
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SW_PRODUCT_IMG_H__ */
