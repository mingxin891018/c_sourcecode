/** 
 * @file swftpfile.h
 * @brief FTP方法下载文件
 * @author ...
 * @date 2007-09-06
 */


#ifndef __FTP_FILE_H__
#define __FTP_FILE_H__

#ifdef __cplusplus
extern "C"
{
#endif 

/** 
 * @brief 初始化
 * 
 * @param url 指向文件下载的URL
 * @param timeout 超时时间
 * 
 * @return 工作句柄,成功; 失败为NULL
 */
HANDLE sw_ftpfile_priv_init( char *url, int timeout );

/** 
 * @brief 退出
 * 
 * @param hfile 工作句柄
 * @param timeout 超时时间
 */
void sw_ftpfile_priv_exit( HANDLE hfile, int timeout );

/** 
 * @brief 打印信息
 */
void sw_ftpfile_priv_print();

/** 
 * @brief 取得ftpclient句柄
 * 
 * @param hfile 
 * 
 * @return ftpclient句柄
 */
HANDLE sw_ftpfile_priv_get_client( HANDLE hfile );

/** 
 * @brief 获取下载文件的大小
 * 
 * @param hfile 工作句柄
 * @param timeout 超时时间
 * 
 * @return 文件的大小,成功;否则, -1
 */
int sw_ftpfile_priv_get_size( HANDLE hfile, int timeout );

/** 
 * @brief 下载文件
 * 
 * @param hfile 工作句柄
 * @param buf 指向存储文件的缓冲区
 * @param size 缓冲区大小
 * @param timeout 超时时间
 * 
 * @return 获取数据的大小,成功; -1,失败    //原来是：return 0,成功; -1,失败
 */
int sw_ftpfile_priv_get_file( HANDLE hfile, char *buf, int size, int timeout );

/** 
 * @brief 上传内容
 * 
 * @param hfile 工作句柄
 * @param buf 指向存储文件的缓冲区
 * @param size 缓冲区大小
 * @param timeout 超时时间
 * 
 * @return 文件大小，成功；-1，失败  //return 0,成功; -1,失败
 */
int sw_ftpfile_priv_upload_file( HANDLE hfile, char *buf, int size, int timeout );



#ifdef __cplusplus
}
#endif

#endif /*__FTP_FILE_H__*/
