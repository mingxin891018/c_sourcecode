#include "swapi.h"
#include "swxml.h"
#include "membuf.h"
#include "swmem.h"

/*================================================================
*   xml_membuf_set_size
*
*   Increases or decreases buffer cap so that at least
*   'new_length' bytes can be stored
*
*   On error, m's fields do not change.
*
*   returns:
*       UPNP_E_SUCCESS
*       UPNP_E_OUTOF_MEMORY
*
*=================================================================*/
static int
xml_membuf_set_size( xml_membuf * m,
                      size_t new_length )
{
	size_t diff;
	size_t alloc_len;
	char *temp_buf;

	if( new_length >= m->length )   // increase length
	{
		// need more mem?
		if( new_length <= m->capacity ) {
			return 0;           // have enough mem; done
		}

		diff = new_length - m->length;
		alloc_len = MAXVAL( m->size_inc, diff ) + m->capacity;
	} else                      // decrease length
	{
		assert( new_length <= m->length );

		// if diff is 0..m->size_inc, don't free
		if( ( m->capacity - new_length ) <= m->size_inc ) {
			return 0;
		}

		alloc_len = new_length + m->size_inc;
	}

	assert( alloc_len >= new_length );

	temp_buf = realloc( m->buf, alloc_len + 1 );
	if( temp_buf == NULL ) {
		// try smaller size
		alloc_len = new_length;
		temp_buf = realloc( m->buf, alloc_len + 1 );

		if( temp_buf == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}
	// save
	m->buf = temp_buf;
	m->capacity = alloc_len;
	return 0;
}

/*================================================================
*   membuffer_init
*
*
*=================================================================*/
void
xml_membuf_init( xml_membuf * m )
{
	assert( m != NULL );

	m->size_inc = MEMBUF_DEF_SIZE_INC;
	m->buf = NULL;
	m->length = 0;
	m->capacity = 0;
}

/*================================================================
*   membuffer_destroy
*
*
*=================================================================*/
void
xml_membuf_destroy( xml_membuf * m )
{
	if( m == NULL ) {
		return;
	}

	free( m->buf );
	xml_membuf_init( m );
}

/*================================================================
*   xml_membuf_assign
*
*
*=================================================================*/
int
xml_membuf_assign( xml_membuf * m,
                    const void *buf,
                    size_t buf_len )
{
	int return_code;

	assert( m != NULL );

	// set value to null
	if( buf == NULL ) {
		xml_membuf_destroy( m );
		return XML_SUCCESS;
	}
	// alloc mem
	return_code = xml_membuf_set_size( m, buf_len );
	if( return_code != 0 ) {
		return return_code;
	}
	// copy
	memcpy( m->buf, buf, buf_len );//已经安全判断了
	m->buf[buf_len] = 0;        // null-terminate

	m->length = buf_len;

	return XML_SUCCESS;

}

/*================================================================
*   xml_membuf_assign_str
*
*
*=================================================================*/
int
xml_membuf_assign_str( xml_membuf * m,
                        const char *c_str )
{
	return xml_membuf_assign( m, c_str, strlen( c_str ) );
}

/*================================================================
*   xml_membuf_append
*
*
*=================================================================*/
int
xml_membuf_append( xml_membuf * m,
                    const void *buf )
{
	assert( m != NULL );

	return xml_membuf_insert( m, buf, 1, m->length );
}

/*================================================================
*   xml_membuf_append_str
*
*
*=================================================================*/
int
xml_membuf_append_str( xml_membuf * m,
                        const char *c_str )
{
	return xml_membuf_insert( m, c_str, strlen( c_str ), m->length );
}

/*================================================================
*   xml_membuf_insert
*
*
*=================================================================*/
int
xml_membuf_insert( xml_membuf * m,
                    const void *buf,
                    size_t buf_len,
                    int index )
{
	int return_code;

	assert( m != NULL );

	if( index < 0 || index > ( int )m->length )
		return XML_INDEX_SIZE_ERR;

	if( buf == NULL || buf_len == 0 ) {
		return 0;
	}
	// alloc mem
	return_code = xml_membuf_set_size( m, m->length + buf_len );
	if( return_code != 0 ) {
		return return_code;
	}
	// insert data
	// move data to right of insertion point
	memmove( m->buf + index + buf_len, m->buf + index, m->length - index );//已经安全判断了
	memcpy( m->buf + index, buf, buf_len );//已经安全判断了
	m->length += buf_len;
	m->buf[m->length] = 0;      // null-terminate

	return 0;
}
