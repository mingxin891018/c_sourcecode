/** 
 * @file swhttpfile.h
 * @brief HTTP方法下载文件
 * @author ...
 * @date 2007-09-06
 */

#ifndef __SWHTTPFILE_H__
#define	__SWHTTPFILE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 初始化http连接下载
 * 
 * @param url 
 * @param timeout 
 * 
 * @return httpfile句柄,成功;失败为NULL
 */
HANDLE sw_httpfile_priv_init( char *url, int timeout );

/** 
 * @brief 退出httpfile
 * 
 * @param h_file 
 * @param timeout 
 */
void sw_httpfile_priv_exit( HANDLE h_file, int timeout );

/** 
 * @brief 打印信息
 */
void sw_httpfile_priv_print();

/** 
 * @brief 取得httpclient句柄
 * 
 * @param h_file 
 * 
 * @return httpclient句柄
 */
HANDLE sw_httpfile_priv_get_client( HANDLE h_file );

/** 
 * @brief 取得HTTP服务器上文件的大小
 * 
 * @param h_file 
 * 
 * @return 
 */
int64_t sw_httpfile_priv_get_size( HANDLE h_file );

/** 
 * @brief 获取服务器文件
 * 
 * @param h_file 
 * @param buf 
 * @param size 
 * @param timeout 
 * 
 * @return 
 */
int sw_httpfile_priv_get_file( HANDLE h_file, char *buf, int size, int timeout );

/** 
 * @brief 取得实际使用的URL，因为有可能重定向了
 * 
 * @param h_file 
 * 
 * @return 
 */
char *sw_httpfile_priv_get_url( HANDLE h_file );

int sw_httpfile_priv_allocmem_getfile( HANDLE h_file, char **p_buf, int *p_size, int timeout );

#ifdef __cplusplus
}
#endif

#endif	/* __SWHTTPFILE_H__ */
