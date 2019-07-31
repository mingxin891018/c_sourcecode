#include "swapi.h"
#include "swlog.h"
#include "swfile_priv.h"
#include "swfile.h"
#include "swcommon_priv.h"
#include <sys/un.h>

swfile_t* sw_file_open( const char *name, const char *mode )
{
	if ( strncasecmp( name, "http://", strlen( "http://" ) ) == 0
#ifdef SUPPORT_HTTPS
			|| strncasecmp( name, "https://", strlen( "https://" ) ) == 0
#endif
			)
		return sw_httpfile_open( name, mode );
#ifdef SUPPORT_FTP
	if ( strncasecmp( name, "ftp://", strlen( "ftp://" ) ) == 0 )
		return sw_ftpfile_open( name, mode );
#endif
	if ( strncasecmp( name, "file://", strlen( "file://" ) ) == 0 )
		name += strlen( "file://" );
	return sw_rawfile_open( name, mode );
}

swfile_t* sw_file_open_ex( const char *name, const char *mode, int timeout, char* extension )
{
	if ( strncasecmp( name, "http://", strlen( "http://" ) ) == 0
#ifdef SUPPORT_HTTPS
			|| strncasecmp( name, "https://", strlen( "https://" ) ) == 0
#endif
			)
		return sw_httpfile_open_ex( name, mode, timeout, extension );
#ifdef SUPPORT_FTP
	if ( strncasecmp( name, "ftp://", strlen( "ftp://" ) ) == 0 )
		return sw_ftpfile_open_ex( name, mode, timeout );
#endif
	if ( strncasecmp( name, "file://", strlen( "file://" ) ) == 0 )
		name += strlen( "file://" );
	return sw_rawfile_open( name, mode ? mode : "rb" );
}

int sw_file_close( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->close( file );
}

int sw_file_shutdown( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	if ( file->shutdown == NULL )
		return -1;
	return file->shutdown( file );
}

int sw_file_read( swfile_t *file, void *buf, int size )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->read( file, buf, size );
}


int sw_file_write( swfile_t *file, void *buf, int size )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->write( file, buf, size );
}


int sw_file_seek( swfile_t *file, int64_t offset, int origin )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->seek( file, offset, origin );
}


int64_t sw_file_tell( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->tell( file );
}


int sw_file_eof( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->eof( file );
}


int sw_file_getc( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->agetc( file );
}


int sw_file_is_seekable( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->is_seekable( file );
}


int64_t sw_file_get_size( swfile_t *file )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	return file->get_size( file );
}

int sw_file_ioctl( swfile_t *file, int cmd,... )
{
	if ( file == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) NULL handle!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	if ( file->ioctl == NULL )
	{
		SWCOMMON_LOG_ERROR( "( %s ) Not support ioctl!!!!!!\n", __FUNCTION__ );
		return -1;
	}
	va_list args;
	int ret;
	va_start(args, cmd);
	ret = file->ioctl( file, cmd, args);
	va_end( args );
	return ret;
}

int sw_file_download(const char *name, int timeout, char* extension, void **filebuf)
{
	if (name == NULL || *name == NULL || filebuf == NULL)
	{
		SWCOMMON_LOG_ERROR("name %s filebuf:%s\n", (name == NULL || *name == '\0') ? "unexist" : "exist", filebuf ? "exist" : "unexist");
		return -1;
	}
	*filebuf = NULL;
	HANDLE fp = sw_file_open_ex(name, "r", timeout, extension);
	int64_t fsize = sw_file_get_size(fp);
	char *p_buf = NULL;
	int tlen = 0;
	if ((fsize <= 0 && fsize != HTTP_CHUNK_SIZE) || (256*1024*1024 < fsize))
	{
		sw_file_close(fp);
		SWCOMMON_LOG_ERROR("file size %lld not validate!!!\n", fsize);
		return -1;
	}
	if (fsize == HTTP_CHUNK_SIZE)
	{
		p_buf = (char *)malloc(1024*1024);
		if (p_buf == NULL)
		{
			sw_file_close(fp);
			return -1;
		}
		
		int rlen = 0;
		int nlen = 1024*1024-1;
		rlen = sw_file_read(fp, p_buf, nlen);
		rlen = (rlen < 0) ? 0 : rlen;
		p_buf[rlen] = '\0';
		tlen = rlen;
		while (rlen == nlen && (tlen < 256*1024*1024))
		{
			char *ptr = (char *)realloc(p_buf, tlen+1024*1024+1);
			if (ptr == NULL)
				break;
			p_buf = ptr;
			nlen = 1024*1024;
			rlen = sw_file_read(fp, &p_buf[tlen], nlen);
			rlen = (rlen < 0) ? 0 : rlen;
			tlen += rlen;
			p_buf[tlen] = '\0';
		}
		sw_file_close(fp);
		if (tlen == 0)
		{
			free(p_buf);
			p_buf = NULL;
		}
	}
	else
	{
		p_buf = (char *)malloc(fsize+1);
		if (p_buf == NULL)
		{
			sw_file_close(fp);
			return -1;
		}
		tlen = sw_file_read(fp, p_buf, fsize);
		tlen = (tlen < 0) ? 0 : tlen;
		p_buf[tlen] = '\0';
		sw_file_close(fp);
		if (tlen == 0)
		{
			free(p_buf);
			p_buf = NULL;
		}
	}
	*filebuf = p_buf;
	return tlen;
}
