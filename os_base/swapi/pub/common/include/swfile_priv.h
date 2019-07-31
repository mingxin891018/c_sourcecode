#ifndef __ABSFILE_PRIV_H__
#define __ABSFILE_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif
#define C2P(childptr, childobj, parent_type) ((parent_type *)((char *)childptr - offsetof(parent_type, childobj)))
typedef struct swfileimpl swfileimpl_t;
struct swfileimpl 
{
    //absfile.c调用时不判断指针有效性. 每个函数都需实现, 无法实现时仅做返回错误的函数
    int     ( *read )( swfileimpl_t *file, void *buf, int size );
    int     ( *write )( swfileimpl_t *file, void *buf, int size );
    int     ( *seek )( swfileimpl_t *file, int64_t offset, int origin );
    int64_t ( *tell )( swfileimpl_t *file );

    int     ( *eof )( swfileimpl_t *file );
    int     ( *agetc )( swfileimpl_t *file );

    int     ( *is_seekable )( swfileimpl_t *file );
    int64_t ( *get_size )( swfileimpl_t *file );

    int     ( *close )( swfileimpl_t *file );
};


swfileimpl_t* sw_rawfile_open( const char *name );

swfileimpl_t* sw_httpfile_open( const char *name );

swfileimpl_t* sw_ftpfile_open( const char *name );

swfileimpl_t* sw_httpfile_open_ex( const char *name, int timeout, char *cookies );

swfileimpl_t* sw_ftpfile_open_ex( const char *name, int timeout );


#ifdef __cplusplus
}
#endif

#endif // __ABSFILE_PRIV_H__

