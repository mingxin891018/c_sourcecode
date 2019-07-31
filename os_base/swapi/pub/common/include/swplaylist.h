/**
*  机顶盒播放列表接口
* 
* <p>
* Copyright (c) 2006 Beijing Sunniwell Broadband Digital Technologies, Inc. All  Rights Reserved.
*
* @author  Dou Hongchen /huanghuaming
* @version 2.0
* @date 2007.02.14
*/

#ifndef __SWPLAYLIST_H__
#define __SWPLAYLIST_H__

/* 服务类型:service_type，参见service_descriptor, 0x80-0xfe是自定义字段，
   为方便直接用了DVB标准中关于service_type的保留字段来定义IP节目 */
#define	SERVICE_TYPE_NULL				0x0	/* 未知 */
#define	SERVICE_TYPE_DIGITAL_TV			0x01/* 数字电视 */
#define	SERVICE_TYPE_DIGITAL_RADIO		0x02/* 数字音频广播 */
#define	SERVICE_TYPE_TELETEXT			0x03/* 图文电视 */
#define	SERVICE_TYPE_NVOD_REFRENCE		0x04/* NVOD服务 */
#define	SERVICE_TYPE_NVOD_TIME_SHIFT	0x05/* NVOD时移 */
#define	SERVICE_TYPE_MOSAIC				0x06/* 马赛克 */
#define	SERVICE_TYPE_PAL_SIGNAL			0x07/* PAL信号 */
#define	SERVICE_TYPE_SECAM_SIGNAL		0x08/* SECAM信号 */
#define	SERVICE_TYPE_D_MAC				0x09
#define	SERVICE_TYPE_FM_RADIO			0x0a/* 调频广播 */
#define	SERVICE_TYPE_NTSC_SIGNAL		0x0b/* NTSC信号 */
#define	SERVICE_TYPE_DATA_BROADCAST		0x0c/* 数据广播 */

/* 下面是自定义类型，注意不要与运营商定义冲突 */
#define	SERVICE_TYPE_USER				0x80/* 自定义类型: 0x80-0xfe */
#define SERVICE_TYPE_IP_AV				(SERVICE_TYPE_USER+1)/* IP视音频频道 */
#define SERVICE_TYPE_IP_PAGE			(SERVICE_TYPE_USER+2)/* IP-WEB页面频道 */

#ifdef __cplusplus
extern "C"
{
#endif


/** 
 * @brief 初始化PLAYLIST
 * 
 * @param size初始化申请内存大小
 *
 * @return true,成功; false,失败
 */
bool sw_playlist_init(int max_buf_size, int max_chnnl_num);

/** 
 * @brief 释放资源
 */
void sw_playlist_exit();

/** 
 * @brief 是否已经初始化
 * 
 * @return true,初始化; false, 未初始化
 */
bool sw_playlist_is_init();

/** 
 * @brief 从文件中载入播放列表
 * 
 * @param file 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_load_from_file( char *file );

/** 
 * @brief 保存内存中的PLAYLIST到file
 * 
 * @param file 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_save_to_file( char *file );

/** 
 * @brief 从FLASH中加载播放列表
 * 
 * @param addr 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_load_from_flash( int addr );

/** 
 * @brief 保存播放列表到FLASH
 * 
 * @param addr 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_save_to_flash( int addr );

/** 
 * @brief 排序播放列表
 * 
 * @param b_descend 
 */
void sw_playlist_sort( bool b_descend );

/** 
 * @brief 读取频道的URL
 * 
 * @param chnl 
 * @param url 
 * @param size 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_get( int chnl, char *url, int size );


/** 
 * @brief 读取频道的URL
 * 
 * 
 * @return 最大的频道号 
 */
int sw_playlist_get_max_chnnl();

/** 
 * @brief 设置频道的URL
 * 
 * @param chnl 
 * @param url 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_set( int chnl, char *url );

/** 
 * @brief 设置频道的URL
 * 
 * @param chnl 
 * @param url 
 * 
 * @return chnl num
 */
int sw_playlist_set_chnl( int chnl, char *url );

/** 
 * @brief 设置一组频道URL
 * 
 * @param buf 
 * @param size 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_set_group( char *buf, int size );

/** 
 * @brief 删除频道，只在缓冲区中操作，调用sw_playlist_Save()保存
 * 
 * @param chnl 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_delete( int chnl );

/** 
 * @brief 清空以前的播放列表，只在缓冲区中操作，调用sw_playlist_Save()保存
 */
void sw_playlist_delete_all();

/** 
 * @brief 取得第一个频道号,返回-1表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_first( unsigned long *pos );

/** 
 * @brief 取得下一个频道号,返回-1表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_next( unsigned long *pos );

/** 
 * @brief 取得上一个频道号,返回-1表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_prev( unsigned long *pos );

/** 
 * @brief 取得最后一个频道号
 * 
 * @param pos 
 * 
 * @return 
 */
int sw_playlist_get_last( unsigned long *pos );

/** 
 * @brief 取得频道号的POS参数
 * 
 * @param chnl 
 * @param pos 
 * 
 * @return true,成功; false,失败
 */
bool sw_playlist_get_pos( int chnl, unsigned long *pos );

/** 
 * @brief 根据POS读取频道
 * 
 * @param pos 
 * @param *url 
 * 
 * @return 
 */
int sw_playlist_get_by_pos( unsigned long pos, char **url );

/* 
 根据频道号获取url
 */
int sw_playlist_get_by_chnl( int chnl,char **url );

/*根据行号获取频道url
 *return 频道对应的chnl
 */
int sw_playlist_get_by_line_num( int line_num,char **url);

 /** 
 * @brief 取得频道个数
 * 
 * @return 
 */
int sw_playlist_get_num();
//根据url查找对应的频道号
int sw_playlist_find_url(char *url);
//根据tsid来获取列表中相同tsid的数目
int sw_playlist_get_url_number(int tsid);
//该函数用来进行匹配的url获取参数进行对比刷新
bool sw_playlist_cmp_channel_url(char *url,int k);

bool sw_playlist_swap(int now,int new);

bool sw_playlist_move(int now,int new);


void sw_playlist_sort_by_name(bool b_descend);


#ifdef __cplusplus
}
#endif

#endif /* __SWPLAYLIST_H__ */
