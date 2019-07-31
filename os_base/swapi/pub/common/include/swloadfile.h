#ifndef __SWLOADFILE_H__
#define __SWLOADFILE_H__

/* 载入文件: 0=success, -1=failed */
int sw_load_file( HANDLE hMem, char *url, char **buf, int *size, int timeout );

#endif /*__SWLOADFILE_H__*/
