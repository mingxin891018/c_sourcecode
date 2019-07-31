#ifndef __ABSFILE_PRIV_H__
#define __ABSFILE_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "swfile.h"

#define C2P(childptr, childobj, parent_type) ((parent_type *)((char *)childptr - offsetof(parent_type, childobj)))

struct swfile 
{
    //absfile.c调用时不判断指针有效性. 每个函数都需实现, 无法实现时仅做返回错误的函数
    int     ( *read )( swfile_t *file, void *buf, int size );
    int     ( *write )( swfile_t *file, void *buf, int size );
    int     ( *seek )( swfile_t *file, int64_t offset, int origin );
    int64_t ( *tell )( swfile_t *file );

    int     ( *eof )( swfile_t *file );
    int     ( *agetc )( swfile_t *file );

    int     ( *is_seekable )( swfile_t *file );
    int64_t ( *get_size )( swfile_t *file );

    int     ( *close )( swfile_t *file );
		int     ( *shutdown )( swfile_t *file );
		
	int		( *ioctl )( swfile_t *file, int cmd, va_list ap);
};


swfile_t* sw_rawfile_open( const char *name, const char *mode );

swfile_t* sw_httpfile_open( const char *name, const char *mode );

swfile_t* sw_ftpfile_open( const char *name, const char *mode );

swfile_t* sw_httpfile_open_ex( const char *name, const char *mode, int timeout, char *cookies );

swfile_t* sw_ftpfile_open_ex(const char *name, const char *mode, int timeout);


#ifdef __cplusplus
}
#endif

#endif // __ABSFILE_PRIV_H__

