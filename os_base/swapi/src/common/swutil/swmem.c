/*************************************************************************
* AUTHOR：hujinshui / huanghuaming
* DATE：2005/09/14
* CONTEXT：可分块/可跟踪内存泄漏位置的内存管理方式
* REMARK：
* HISTORY: 
*************************************************************************/
#include "swapi.h"
#include "swmem.h"
#include "swthrd.h"
#include "swlog.h"
#include "swmutex.h"
#include "swutil_priv.h"

#define _DEBUG


//整数对齐
#define ALIGN_SIZE(size)	((size+align) & ~align)		//这里的align：xm->align-1
//指针((char*)p+size)对齐
#define ALIGN_PTR(p,size)	((size+align+(unsigned long)p) & ~align)

#define MEM_CHECK_SIZE 8
static const char m_memdbg_flag[] = "\xa5\xc1\xd8\x8e\xf1\xbd\x97\xec\xa6\xc2\xd9\x8f\xf2\xbe\x96\xed";

/*
static const char *GetPathFile( const char *path )
{
	int n = strlen(path)-1;
	while( 0 < n )
	{
		if( path[n] == '/' || path[n] == '\\' )
			return path+n+1;
		n--;
	}
	return path;
}
*/

/////////////////////////////////////////////////////////////////////////
typedef struct _MemHeader  
{
#ifdef _DEBUG
	//char filename[16];
	const char* filename;
	int line;
#endif
	int size;
	struct _MemHeader *prev, *next;
}smemnode_t;

typedef struct
{
	//可支配的缓冲区
	char *buf;
	//可支配的缓冲区大小
	int size;
	//对齐字节数
	int align;
	//当前已经分配的内存大小
	int alloc_size;
#ifdef _DEBUG
	//最大分配位置
	int maxpos;
#endif

	smemnode_t *first, *tail;	//其后有空闲块的结点，first是最前的空闲块结点，tail是最后的空闲块结点
	HANDLE mutex;
	
	//分配链表
	smemnode_t *list;
}sxmem_t;


//////////////////////////////////////////////////////////////////////////////////

/***
 * @brief 对已分配的内存进行初始化
 * 
 * @param buf 	已分配的内存地址
 * @param size 	已分配内存的大小 
 * @param align 字节分配的方式
 * @use_mutex   使用互斥量，如果此句柄仅由单线程访问，此值可置0，否则请置1
 * @return 成功,返回内存句柄; 失败,返回空值
 */
HANDLE sw_mem_init_ex( char *buf, int size, int align, int use_mutex )
{
	if (buf == NULL || size < (int)sizeof(sxmem_t))
		return NULL;
	sxmem_t *xm = (sxmem_t *)(((align-1+(unsigned long)buf)/align)*align);

	memset( xm, 0, sizeof(sxmem_t) );
	xm->align = align;
	align--;
	xm->buf = (char *)ALIGN_PTR( xm, sizeof(sxmem_t) );		//可分配内存的起点
	xm->size = buf + size - xm->buf;						//可分配内存的大小
	if( use_mutex )
		xm->mutex = sw_mutex_create();
	
	return xm;
}


/***
 * @brief 对已分配的内存进行初始化 
 * 
 * @param buf 	已分配的内存地址
 * @param size 	已分配内存的大小 
 * @param align 字节分配的方式
 * 
 * @return 成功,返回内存句柄; 失败,返回空值
 */
HANDLE sw_mem_init( char *buf, int size, int align )
{
	return sw_mem_init_ex( buf, size, align, 1 );
}


/***
 * @brief 释放内存句柄
 * 
 * @param h 内存句柄
 */
void sw_mem_exit( HANDLE h )
{
	if( h==NULL )
		return;
#ifdef _DEBUG
	sw_mem_check( h );
#endif
	sw_mutex_destroy( ((sxmem_t *)h)->mutex );
}

/***
 * @brief 释放内存句柄, 不做内存泄露检查, 提高执行效率
 * 
 * @param h 内存句柄
 */
void sw_mem_exit_nocheck( HANDLE h )
{
	if( h==NULL )
		return;
#ifdef _DEBUG
	sw_mem_check_ex( h, 0 );
#endif
	sw_mutex_destroy( ((sxmem_t *)h)->mutex );
}

/***
 * @brief 获取总的内存大小
 *
 * @param h 内存句柄
 * @return 总的内存大小
 */
int sw_mem_get_total_size( HANDLE h )
{
	return h ? ((sxmem_t *)h)->size : 0;
}

/***
 * @brief 内存清空，强制释放掉所有已分配的内存
 * @param h 内存句柄
 */
void sw_mem_reset( HANDLE h )
{
	if (h == NULL)
		return;
	((sxmem_t *)h)->list = ((sxmem_t *)h)->first = ((sxmem_t *)h)->tail = NULL;
}


/***
 *   将空闲内存中的数据全部设置为0
 * @param h 内存句柄
 */
void sw_mem_zero( HANDLE h )
{
	sxmem_t* xm = (sxmem_t*)h;
	if( xm==NULL )
		return;

	sw_mutex_lock( xm->mutex );
	if( xm->list==NULL )
	{
		memset( xm->buf, 0, xm->size );
	}
	else
	{
		smemnode_t *tail;
		unsigned int align = xm->align-1;
		char* p;
		tail = xm->tail;
		p = (char*)ALIGN_PTR( tail, sizeof(smemnode_t) );	//最后那个结点的buf
		p = (char *)ALIGN_PTR( p, tail->size );		//最后那个结点的buf的尾，可以作为下一结点地址
		memset( p, 0, xm->buf + xm->size - p );
	}
	sw_mutex_unlock( xm->mutex );
}

/***
 * @brief 取历史上最大已分配尺寸
 * @param h 内存句柄
 * 
 * @return 最大已分配尺寸的大小
 */
int sw_mem_get_max_size( HANDLE h )
{
#ifdef _DEBUG
	return ((sxmem_t *)h)->maxpos;
#else
	return sw_mem_get_cur_size( h );
#endif
}

/***
 * @brief 取现在最大分配尺寸
 * @param h 内存句柄
 * @return 现在分配尺寸的大小
 */
int sw_mem_get_cur_size( HANDLE h )
{
	if (h == NULL)
		return 0;
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node;
	char *p;
	unsigned int align = xm->align-1;
	int size = 0;
	if( xm->tail )
	{
		node = xm->tail;
		p = (char*)ALIGN_PTR(node, sizeof(smemnode_t));
		size = p-xm->buf + node->size;
	}
	
	return size;
}


static inline int crash_len( char* buf )
{
	int i;
	for( i=MEM_CHECK_SIZE; i>0 && buf[i-1] == m_memdbg_flag[i-1]; i-- );
	return i;
}


/** 
 * @brief 检查是否有未释放的内存，以及内存键表是否有错误 
 * 
 * @param h 内存句柄
 * @param print_node: 是否打印所有内存结点
 */
void sw_mem_check_ex( HANDLE h, int print_node )
{
	if (h == NULL)
		return ;
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node = xm->list;
	int node_count, used_size, available_size, max_available;
	char *p, *end;
	int available;
	unsigned int align = xm->align-1;

	node_count = 0;
	used_size = 0;
	available_size = 0;
	max_available = 0;

	if( xm->list )
	{
		available = (unsigned long)xm->list - ALIGN_PTR(xm->buf,sizeof(smemnode_t));
		if( available > 0 )
		{
			if( max_available < available )
				max_available = available;
			available_size += available;
		}
		for( node = xm->list; node; node = node->next )
		{
			//检查内存指针是否有越界
			if( (unsigned long)node < (unsigned long)xm
				|| (unsigned long)node >= (unsigned long)(xm->buf + xm->size)
				|| (unsigned long)ALIGN_PTR(node,sizeof(smemnode_t)) + (unsigned long)node->size > (unsigned long)(xm->buf + xm->size)
				|| node->size < 0
				|| (node != xm->list && (node->prev < xm->list || node->prev >= node || node->prev->next != node))
				|| (node->next && (node->next <= node || (char*)node->next > xm->buf + xm->size || node->next->prev != node)) )
			{
				if( (unsigned long)node < (unsigned long)xm
					|| (unsigned long)node >= (unsigned long)(xm->buf + xm->size) )
				{
					UTIL_LOG_DEBUG( "sw_mem_check error: node=%p, xm=%p, end=%p, xm->size=0x%x, node(size=%d, file=%s, line=%d)\n",
						node, xm, xm->buf+xm->size, xm->size, 0, "", 0 );
				}
				else
				{
					UTIL_LOG_DEBUG( "sw_mem_check error: node=%p, xm=%p, end=%p, xm->size=0x%x, node(size=%d, file=%s, line=%d)\n",
						node, xm, xm->buf+xm->size, xm->size, node->size-MEM_CHECK_SIZE, node->filename, node->line );
				}
			}

			used_size += node->size;
			if( node->next )
			{
				end = (char*)ALIGN_PTR(node,sizeof(smemnode_t)) + ALIGN_SIZE(node->size);
				available = ((char*)node->next) - (char*)ALIGN_PTR(end, sizeof(smemnode_t));
			}
			else
			{
				p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );
				end = (char *)ALIGN_PTR( p, node->size );
				p = (char*)ALIGN_PTR( end, sizeof(smemnode_t) );
				available = ALIGN_SIZE( xm->buf+xm->size - p );
			}
			if( available > 0 )
			{
				if( max_available < available )
					max_available = available;
				available_size += available;
			}
			node_count ++;
		}
	}
	else
	{
		p = (char*)ALIGN_PTR( xm->buf, sizeof(smemnode_t) );
		available_size = ALIGN_SIZE( xm->buf+xm->size - p );
		max_available = available_size;
	}

#if MEM_CHECK_SIZE > 0
	for( node = xm->list; node; node = node->next )
	{
		p = (char*)ALIGN_PTR(node,sizeof(smemnode_t));
		if( memcmp((char*)p + node->size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE)!=0 )
		{
			UTIL_LOG_ERROR( "%s(%p, size=%d-->%d, from %s:%d) memory is crashed.\n", __FUNCTION__, p, node->size-MEM_CHECK_SIZE, crash_len((char*)p + node->size-MEM_CHECK_SIZE), node->filename, node->line );
		}
	}
#endif
	if( print_node )
	{
#ifdef _DEBUG
		for( node = xm->list; node; node = node->next )
		{
			UTIL_LOG_DEBUG( "===>MEMORY LEAK: %s, line %d, %d bytes\n", node->filename, node->line, node->size-MEM_CHECK_SIZE );
		}
		UTIL_LOG_DEBUG( "MAX_POS=%d\n", xm->maxpos );
#endif

		UTIL_LOG_DEBUG( "%d nodes, %d bytes allocated.\tmemory %p, %d total bytes, %d bytes available, %d bytes max block\n", node_count, used_size, h, xm->size, available_size, max_available );
	}
}

/** 
 * @brief 检查是否有未释放的内存，以及内存键表是否有错误 
 * 
 * @param h 内存句柄
 */
void sw_mem_check( HANDLE h )
{
	sw_mem_check_ex( h, 1 );
}


#define INSERT_BEFORE(node, nex) {node->next=nex;if(nex){node->prev=nex->prev;nex->prev=node; if(node->prev)node->prev->next=node;}else{node->prev=NULL;}}
#define INSERT_AFTER(node, pre) {node->prev=pre;if(pre){node->next=pre->next;pre->next=node; if(node->next)node->next->prev=node;}else{node->next=NULL;}}
#define REMOVE_NODE(node) {if(node->prev)node->prev->next=node->next; if(node->next)node->next->prev=node->prev;}


/** 
 * @brief	扫描链表看是否有free块的内存符合申请的要求 
 * 
 * @param h	内存句柄
 * @param size	已分配内存的大小
 * @param filename	所在的当前文件名
 * @param line	所在的当前行号
 * 
 * @return 成功返回分配后内存的起始地址; 否则,返回空值？？？？（不确定）
 */
static void* malloc_old( HANDLE h, int size ,const char *filename, int line)
{
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node, *new_node;
	unsigned int align = xm->align-1;
	char *end;
	int len;
	int firstlen;	//第一个空闲块的大小

	//检查头和buf之间的空隙;
	if( xm->first==NULL )
	{
		if( xm->list == NULL )
			return NULL;
		
		node = xm->list;
		len = ((unsigned long)node) - ALIGN_PTR(xm->buf,sizeof(smemnode_t));
		if( size <= len )
		{
			//填充NODE
			new_node = (smemnode_t *)xm->buf;
			memset( new_node, 0, sizeof(smemnode_t) );
#ifdef _DEBUG
			//strncpy( new_node->filename, GetPathFile(filename), sizeof(new_node->filename)-1 );
			new_node->filename = filename;
			new_node->line = line;
#endif
			new_node->size = size;
			
			//插入到链表头;
			xm->list = new_node;
			//new_node->next = node;
			INSERT_BEFORE( new_node, node );
			xm->first = new_node;
			return (char*)ALIGN_PTR(new_node, sizeof(smemnode_t)); 
		}
		firstlen = len;
	}
	else
	{
		node = xm->first;
		firstlen = 0;
	}
	
	//检查相临两块之间的间隙是否大于下一块的长度;
	while( node->next )
	{
		end = (char*)ALIGN_PTR(node,sizeof(smemnode_t)) + ALIGN_SIZE(node->size);
		len = ((char*)node->next) - (char*)ALIGN_PTR(end, sizeof(smemnode_t));
		
#if MEM_CHECK_SIZE > 0
		if( firstlen <= (int)align + ((MEM_CHECK_SIZE+3)&~3) )	//如果第一空闲块太小，不够最低分配字节数，就修改这个指针，
#else
		if( firstlen <= (int)align )	//如果第一空闲块太小，不够最低分配字节数，就修改这个指针，
#endif
		{
			xm->first = node;
			firstlen = len;
		}
		
		//找到符合要求的块;
		if( size <= len )
		{
			//填充NODE
			new_node = (smemnode_t *)end;
			memset( new_node, 0, sizeof(smemnode_t) );
#ifdef _DEBUG
			//strncpy( new_node->filename, GetPathFile(filename), sizeof(new_node->filename)-1 );
			new_node->filename = filename;
			new_node->line = line;
#endif
			new_node->size = size;
			
			//插入到链表;
			//new_node->next = node->next;
			//node->next = new_node;
			INSERT_AFTER( new_node, node );
			
			return (char*)ALIGN_PTR(new_node, sizeof(smemnode_t)); 
		}
		node=node->next;
		//加入到分配链表;
	}
	return NULL;
}


/** 
 * @brief 从内存句柄所指向的内存中分配一段内存
 * 
 * @param h 内存句柄
 * @param size 分配内存的大小
 * @param filename 所在的当前文件名 
 * @param line 所在的当前行号
 * 
 * @return 成功返回分配后内存的地址; 否则,返回空值
 */
void *sw_mem_alloc( HANDLE h, int size, const char *filename, int line )
{
	if (h == NULL)
		return NULL;
	sxmem_t *xm = (sxmem_t*)h;
	smemnode_t *tail = NULL;
	smemnode_t *node;
	unsigned int align = xm->align-1;
	char *p;

	if( size<0 )
	{
		UTIL_LOG_DEBUG( "%s error  %s:%d size=%d\n", __FUNCTION__, filename, line, size );
		return NULL;
	}
	size += MEM_CHECK_SIZE;
	sw_mutex_lock( xm->mutex );
	//从free块分配
	p = (char *)malloc_old( h, size, filename, line );
	if( p )
	{
#ifdef _DEBUG
		if( xm->maxpos < (p-xm->buf) + size )
			xm->maxpos = (p-xm->buf) + size;
#endif

		xm->alloc_size += (size+(int)sizeof(smemnode_t));//是否需要size对齐?	
		sw_mutex_unlock( xm->mutex );
#if MEM_CHECK_SIZE > 0
		memcpy((char*)p+size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE);
#endif
		return p;
	}
	
	//第一次申请
	if( xm->list == NULL )
		node = (smemnode_t *)xm->buf;
	else
	{
		tail = xm->tail;
		
		p = (char*)ALIGN_PTR( tail, sizeof(smemnode_t) );	//最后那个结点的buf
		node = (smemnode_t *)ALIGN_PTR( p, tail->size );	//最后那个结点的buf的尾，可以作为下一结点地址
	}
	
	p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );	//获取结点的buf
	
	//检查是否有足够的内存
	if( xm->buf+xm->size < p+size )
	{
		if( xm->mutex != NULL )
		{
			UTIL_LOG_DEBUG("no enough memory h=%p, size=%d at %s %d\n", h, size-MEM_CHECK_SIZE, filename, line);
			sw_mem_check(h);
		}
		sw_mutex_unlock( xm->mutex );
		return NULL;
	}
	
	//填充node
	memset( node, 0, sizeof(smemnode_t) );
	node->size = size;
#ifdef _DEBUG
	//strncpy( node->filename, GetPathFile(filename), sizeof(node->filename)-1 );
	node->filename = filename;
	node->line = line;
#endif
	
	//把当前节点挂到链表尾部
	if( tail == NULL )
		xm->list = node;
	else
		tail->next = node;
#ifdef _DEBUG
	if( xm->maxpos < (p-xm->buf) + size )
		xm->maxpos = (p-xm->buf) + size;
#endif
	
	node->prev = tail;
	xm->tail = node;
	xm->alloc_size += (size+(int)sizeof(smemnode_t));//是否需要size对齐?
	
	sw_mutex_unlock( xm->mutex );
#if MEM_CHECK_SIZE > 0
	memcpy((char*)p+size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE);
#endif
	return p;
}

/** 
 * @brief 在原有内存的基础上重新申请内存
 * 
 * @param h 内存句柄
 * @param p 指向原有的内存
 * @param size 分配内存的大小
 * @param filename 所在的当前文件名
 * @param line 所在的当前行号
 * 
 * @return 成功,返回实际分配后的新地址; 否则,返回空值
 */
void *sw_mem_realloc( HANDLE h, void *p, int size, const char *filename, int line )
{
	if (h == NULL)
		return NULL;
	void *buf = NULL;
	sxmem_t *xm = (sxmem_t*)h;
	smemnode_t* node = NULL;
	unsigned int align = xm->align-1;
	int oldsize = 0;
	//定位;
	if( p )
	{
		size += MEM_CHECK_SIZE;
		sw_mutex_lock( xm->mutex );
		if( xm->buf<(char*)p && (char*)p<xm->buf+xm->size )
		{
			node = (smemnode_t*)(((unsigned long)p - sizeof(smemnode_t))&~align);
			oldsize = node->size;
		}
		if( node &&
			(size<=node->size ||
			(char*)ALIGN_PTR(node,sizeof(smemnode_t))+size <= ( node->next ? (char*)node->next : xm->buf + xm->size) ) )
		{
#if MEM_CHECK_SIZE > 0
			if( memcmp((char*)p + node->size - MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE)!=0 )
			{
				UTIL_LOG_ERROR( "%s:%d  %s(%p, size=%d-->%d from %s:%d) xm=%p, memory is crashed.\n", filename, line, __FUNCTION__, p, node->size-MEM_CHECK_SIZE, crash_len((char*)p + node->size - MEM_CHECK_SIZE), node->filename, node->line, xm );
			}
#endif
			node->size = size;

			xm->alloc_size = (xm->alloc_size-oldsize+size);
			sw_mutex_unlock( xm->mutex );
#if MEM_CHECK_SIZE > 0
			memcpy((char*)p+size-MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE);
#endif
			return p;
		}
		sw_mutex_unlock( xm->mutex );
		size -= MEM_CHECK_SIZE;
	}
	
	//内存不够,重新分配;
	buf = sw_mem_alloc( h, size, filename, line );
	if( buf && p )
	{
		//拷贝内容;
		memcpy( buf, p, (node && node->size < size && node->size-MEM_CHECK_SIZE >= 0) ? node->size-MEM_CHECK_SIZE : size );
		//释放原来的内存;
		sw_mem_free( h, p, filename, line );
	}
	
	return buf;
}

/** 
 * @brief 从内存句柄指向的内存中复制字符串
 * 
 * @param h 内存句柄
 * @param s 指向的字符串
 * @param filename 所在的当前文件名
 * @param line 所在的当前行号
 * 
 * @return 成功,返回复制后的字符串指针; 否则,返回空值
 */
char *sw_mem_strdup( HANDLE h, const char *s, const char *filename, int line )
{
	if (h == NULL || s == NULL)
		return NULL;
	int len = strlen(s);
	char *p = (char*)sw_mem_alloc( h, len+1, filename, line );
	if( p )
		memcpy(p, s, len+1);
	return p;
}



/** 
 * @brief 			释放内存句柄所指向的一段内存
 * 
 * @param h 		内存句柄
 * @param p 		指向的内存地址
 * @param filename 	所在的当前文件名
 * @param line 		所在的当前行号
 */
void sw_mem_free( HANDLE h, void *p, const char *filename, int line )
{
	if (h == NULL)
		return;
	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node = NULL;
	unsigned int align = xm->align-1;
	int freesize = 0;

	sw_mutex_lock( xm->mutex );
	if( xm->buf<(char*)p && (char*)p<xm->buf+xm->size )
	{
		node = (smemnode_t*)(((unsigned long)p - sizeof(smemnode_t))&~align);
		freesize = node->size+(int)sizeof(smemnode_t);
	}
	//释放一块不存在的内存
	if( node == NULL || xm->list == NULL )
	{
		UTIL_LOG_DEBUG( "sw_mem_free error: %s:%d, memory %p\n", filename, line, p );
	}
	else
	{
#ifdef _DEBUG
		char* end = xm->buf + xm->size;
		if( node->size < 0
			|| (char*)p + node->size > end
			|| (node > xm->list && (node->prev < xm->list || node->prev >= node || node->prev->next != node))
			|| (node->next && (node->next <= node || (char*)node->next > end || node->next->prev != node)) )
		{
			UTIL_LOG_DEBUG( "sw_mem_free error: %s:%d, memory %p is damaged.\n", filename, line, p );
			sw_mem_check_ex( xm, 1 );
			sw_mutex_unlock( xm->mutex );
			return;
		}
#endif
#if MEM_CHECK_SIZE > 0
		if( memcmp((char*)p + node->size - MEM_CHECK_SIZE, m_memdbg_flag, MEM_CHECK_SIZE)!=0 )
		{
			UTIL_LOG_ERROR( "%s:%d  %s(%p, size=%d-->%d, from %s:%d) xm=%p, memory is crashed.\n", filename, line, __FUNCTION__, p, node->size-MEM_CHECK_SIZE, crash_len((char*)p + node->size - MEM_CHECK_SIZE), node->filename, node->line, xm );
		}
#endif
		REMOVE_NODE(node);

		if( xm->list == node )
			xm->list = node->next;
		if( xm->first >= node )
			xm->first = node->prev;
		if( xm->tail == node )
			xm->tail = node->prev;
	}
	xm->alloc_size -= freesize;
	sw_mutex_unlock( xm->mutex );
}

/** 
 * @brief 返回内存是否在使用中(有分配过单没释放的内存)
 * 
 * @param h 内存句柄
 * 
 * @return 如果内存没有使用,则返回真(true);否则,返回假(false)
 */
bool sw_mem_is_empty( HANDLE h )
{
	if (h == NULL)
		return false;
	sxmem_t *xm = (sxmem_t *)h;
	return xm->list == NULL;
}

/***
 *   获取已分配内存的内存块大小
 * @param h: 内存句柄
 * @param p: 由 sw_mem_alloc 或 sw_mem_realloc 返回的内存指针
 */
int sw_mem_get_size( HANDLE h, void* p )
{
	if (h == NULL || p == NULL)
		return 0;
	sxmem_t* xm = (sxmem_t*)h;
	smemnode_t* node = NULL;
	unsigned int align = xm->align-1;
	if( xm->buf<(char*)p && (char*)p<xm->buf+xm->size )
	{
		node = (smemnode_t*)(((unsigned long)p - sizeof(smemnode_t))&~align);
		return node->size-MEM_CHECK_SIZE;
	}
	return 0;
}

/***
 *   获取已分配内存总大小
 * @param h: 内存句柄
 */
int sw_mem_get_alloc_size( HANDLE h )
{
	if (h == NULL)
		return 0;
	sxmem_t* xm = (sxmem_t*)h;
	return xm->alloc_size;
}