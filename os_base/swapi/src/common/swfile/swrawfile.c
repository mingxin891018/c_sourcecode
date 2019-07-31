#include <stddef.h>
#include "swapi.h"
#include "swlog.h"
#include "swfile_priv.h"
#include "swcommon_priv.h"


typedef struct swrawfile
{
    swfile_t impl;
    FILE *fp;
} swrawfile_t; 


static int sw_rawfile_read( swfile_t *file, void *buf, int size )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    return fread( buf, 1, size, raw->fp );
}


static int sw_rawfile_write( swfile_t *file, void *buf, int size )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    return fwrite( buf, 1, size, raw->fp );
}


static int sw_rawfile_seek( swfile_t *file, int64_t offset, int origin )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    return fseeko( raw->fp, offset, origin );
}


static int64_t sw_rawfile_tell( swfile_t *file )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    return ftello( raw->fp );
}


static int sw_rawfile_eof( swfile_t *file )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    return feof( raw->fp );
}


static int sw_rawfile_getc( swfile_t *file )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    return fgetc( raw->fp );
}


static int sw_rawfile_is_seekable( swfile_t *file )
{
    SW_UNUSED( file );
    return 0;
}


static int64_t sw_rawfile_get_size( swfile_t *file )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    int64_t posbak = ftello( raw->fp );
    int64_t size;
    fseeko( raw->fp, 0, SEEK_END );
    size = ftello( raw->fp );
    fseeko( raw->fp, posbak, SEEK_SET );
    return size;
}


static int sw_rawfile_close( swfile_t *file )
{
    swrawfile_t *raw = C2P( file, impl, swrawfile_t );
    SWCOMMON_LOG_INFO( "( %s ) %p\n", __FUNCTION__, file );
    fclose( raw->fp );
    free( raw );
    return 0;
}


swfile_t* sw_rawfile_open( const char *name, const char *mode )
{
    swrawfile_t *raw = NULL;

    raw = ( swrawfile_t * )malloc( sizeof( swrawfile_t ) );
    if ( raw == NULL )
        goto FAIL;
    memset( raw, 0, sizeof( swrawfile_t ) );

    //SWCOMMON_LOG_INFO( "( %s %p) %s\n", __FUNCTION__, raw , name);

    raw->fp = fopen( name, mode );
    if ( raw->fp == NULL )
        goto FAIL;

    raw->impl.read  = sw_rawfile_read;
    raw->impl.write = sw_rawfile_write;
    raw->impl.seek  = sw_rawfile_seek;
    raw->impl.tell  = sw_rawfile_tell;
    raw->impl.eof   = sw_rawfile_eof;
    raw->impl.agetc = sw_rawfile_getc;
    raw->impl.is_seekable = sw_rawfile_is_seekable;
    raw->impl.get_size    = sw_rawfile_get_size;
    raw->impl.close       = sw_rawfile_close;

    return &raw->impl;

FAIL:
    if ( raw != NULL )
    {
        if ( raw->fp != NULL )
            fclose( raw->fp );
        free( raw );
    }
    return NULL;
}

