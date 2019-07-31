/**
 * @file swparameter.c
 * @brief 实现参数管理接口
 * @author Dou Hongchen / huanghuaming
 * @history 2007-02-14 created
 *			2011-01-20 liuchunrui 实现新的参数方案
 *			2011-02-28 chenkai 对代码进行了重新整理
 */
#include "swapi.h"
#include "swlog.h"
#include "swmem.h"
#include "swmutex.h"
#include "swthrd.h"
#include "swsignal.h"
#include "swparameter.h"

/* 最大参数个数 */
#define MAX_PARA_NUM		2048
/*最大参数仓库的个数*/
#define MAX_DEPOT_NUM		8
/*参数观察者的最大数目*/
#define MAX_OBSERVER_NUM    4

typedef struct
{
	char* name;
	char* value;
	bool  readonly;
	bool	realtime;	/* 实时更新 */
	sw_idepot_t* depot;
}para_desc_t;

/* 参数改变回调函数*/
typedef struct
{
	on_para_modified  on_modified;
	void* handle;
}para_observer_t;

typedef struct
{
	char* buf;
	HANDLE mem;
	HANDLE thrd; //参数保存线程
	HANDLE signal;
	HANDLE mutex;
	uint32_t timeout;
	sw_idepot_t* depots[MAX_DEPOT_NUM]; //参数仓库数组
	sw_idepot_t* defdepot;	//当没有指定参数仓库时，增加的参数存在这个仓库里
	int dcount;	//实际仓库的数目
	para_observer_t observers[MAX_OBSERVER_NUM]; //注册的参数值改变的回调函数
	int ocount;	//回调函数的数目
  para_desc_t plist[MAX_PARA_NUM]; //参数列表
	int pnum;	//参数列表中参数的数目		
	bool exit;	//是否退出保存线程
	bool saveall;//保存所有变更的参数
	bool saveauto;//保存自动保存的参数
}sw_parameter_t;

static sw_parameter_t  m_parameter;

/* 查找参数所在的序号 */
static int sw_parameter_find( char *name );
/*通过仓库名查找参数仓库*/
static int depot_find_byname(char* name);
/*增加一个参数*/
static bool sw_parameter_set_with_depot(char* name,char* value,sw_idepot_t* depot,bool is_load,bool only_default);
//参数保存线程
static bool param_save_proc(uint32_t wparam,uint32_t lparam);
static void insert_qsort();

/** 
 * @brief 初始化参数配置
 * 
 * @param max_buf_size 参数模块缓冲区的大小，推荐值是128*1024
 *
 * @return true,成功; false, 失败
 */
bool sw_parameter_init(int max_buf_size)
{
	memset(&m_parameter,0,sizeof(sw_parameter_t));
	m_parameter.buf  = (char*)malloc(max_buf_size);

	if( m_parameter.buf  == NULL || max_buf_size <=0  )
		goto ERROR_EXIT;

	m_parameter.mem = sw_mem_init(m_parameter.buf,max_buf_size,4);
	if(m_parameter.mem == NULL)
		goto ERROR_EXIT;

	m_parameter.timeout = 300;			

	m_parameter.mutex = sw_mutex_create();
	m_parameter.signal = sw_signal_create(0,1);

	if( m_parameter.mutex == NULL  || m_parameter.signal == NULL)	
		goto ERROR_EXIT;

	m_parameter.thrd = sw_thrd_open( "tParaSaveProc", 80, 
                     	SW_SCHED_NORMAL,64 * 1024,param_save_proc,(uint32_t)(&m_parameter),0);

	if(m_parameter.thrd == NULL )
		goto ERROR_EXIT;
	
	sw_thrd_resume(m_parameter.thrd);

	return true;
ERROR_EXIT:
	if( m_parameter.thrd != NULL )
	{
		sw_thrd_close(m_parameter.thrd,300);
		m_parameter.thrd = NULL;
	}
	
	if(m_parameter.signal!= NULL )
	{
		sw_signal_destroy(m_parameter.signal);
		m_parameter.signal = NULL;
	}

	if(m_parameter.mutex != NULL )
	{
		sw_mutex_destroy(m_parameter.mutex);
		m_parameter.mutex = NULL;
	}

	if( m_parameter.mem)
	{
		sw_mem_exit(m_parameter.mem);
		m_parameter.mem = NULL;
	}

	if(m_parameter.buf )
	{
		free(m_parameter.buf);
		m_parameter.buf = NULL;
	}

	printf("sw_parameter_init failed ..\n");
	return false;
}

/** 
 * @brief 是否已经初始化
 * 
 * @return true,初始化; false, 未初始化
 */
bool sw_parameter_is_init()
{
	return ( m_parameter.mem != NULL );
}

/** 
 * @brief 释放资源
 */
void sw_parameter_exit()
{
	m_parameter.exit = true;
	if( m_parameter.signal != NULL )
		sw_signal_give(m_parameter.signal);
	
	sw_thrd_delay(50);
		
	if( m_parameter.thrd != NULL )
	{
		sw_thrd_close(m_parameter.thrd,400);
		m_parameter.thrd = NULL;
	}
	
	if(m_parameter.signal!= NULL )
	{
		sw_signal_destroy(m_parameter.signal);
		m_parameter.signal = NULL;
	}

	if(m_parameter.mutex != NULL )
	{
		sw_mutex_destroy(m_parameter.mutex);
		m_parameter.mutex = NULL;
	}

	if( m_parameter.mem)
	{
		sw_mem_exit_nocheck(m_parameter.mem);
		m_parameter.mem = NULL;
	}

	if(m_parameter.buf )
	{
		free(m_parameter.buf);
		m_parameter.buf = NULL;
	}
}


/**
 *	@brief 注册参数仓库,注册之后参数仓库里的函数通过遍历接口加载到parameter里
 */
//参数加载的处理函数
static int on_spread_para(sw_idepot_t *depot,char* name,char* value)
{
	sw_parameter_set_with_depot(name,value,depot,true,false);
	return 0;
}

bool sw_parameter_register_depot(sw_idepot_t* depot,bool isdefault)
{
	int  i = 0;

	if( depot == NULL )
		return false;
	
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	//如果depot指针相同则不重复加入
	for( i=0;i<m_parameter.dcount;i++)
	{
		if( m_parameter.depots[i] == depot )
			break;
	}
	if( i < m_parameter.dcount) 
	{
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		return true;
	}

	//数组满则返回
	if(m_parameter.dcount >= MAX_DEPOT_NUM )
	{
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		return false;
	}
	
	i = m_parameter.dcount;
	m_parameter.depots[i] = depot;
	m_parameter.dcount++;
	if( isdefault || m_parameter.defdepot == NULL )
		m_parameter.defdepot = depot;
	
	if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);

	depot->load(depot,on_spread_para);
	return true;
}

/**
 * @breif 根据名称获取参数仓库
 */
sw_idepot_t* sw_parameter_get_depot(char* name)
{
	int t = -1;
	sw_idepot_t* depot = NULL;

	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	t =  depot_find_byname(name);
	if( t >= 0 && t< m_parameter.dcount )
		depot = m_parameter.depots[t];
	
	if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
	
	return depot;
}

/**
 * @brief 卸载参数仓库，删掉paraemeter里存在该仓库里的参数
 */
bool sw_parameter_unregister_depot( char* name )
{
	int i,j,t, maxnum;
	sw_idepot_t* depot = NULL;
	
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	t =  depot_find_byname(name);
	if( t >= 0 && t< m_parameter.dcount )
	{
		depot = m_parameter.depots[t];
		if( t + 1< m_parameter.dcount) 
			memmove(m_parameter.depots+t,m_parameter.depots+t+1,sizeof(sw_idepot_t)*(m_parameter.dcount-1-t) );
		m_parameter.depots[m_parameter.dcount-1] = NULL;
		m_parameter.dcount--;
	}
	
	if( depot == NULL )
	{	
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		return true;
	}
	
	//删除所有在该仓库里的参数
	maxnum = m_parameter.pnum;
	j=0;
	for( i=0; i < maxnum; i++ )
	{
		if(  m_parameter.plist[i].depot !=  depot )
		{
			if ( j != i )
			{
				memmove( m_parameter.plist+j, m_parameter.plist+i, sizeof(para_desc_t));
				memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
			}
			j++;
		}
		else
		{
			if( m_parameter.plist[i].name )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].name, __FILE__, __LINE__ );
			if( m_parameter.plist[i].value )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
	
			memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
		}
	}
	m_parameter.pnum = j;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return true;
}


/*
 * @brief 将指定的depot数据分发到swparameter中
 * @param sw_paradepot_t* p_depot, 更新所需要的depot
 * @return 成功返回true,失败返回false
 */
//参数更新的处理函数
static int on_update_para(sw_idepot_t *depot,char* name,char* value)
{
	sw_parameter_set_with_depot(name,value,NULL,false,false);
	return 0;
}
bool sw_parameter_updatefrom_depot( sw_idepot_t* depot )
{
	if( depot )
	{
		depot->load(depot,on_update_para);
		return true;
	}
	else
		return false;
}


/** 
 * @brief 保存机顶盒参数, 保存目标由load方式决定
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_save()
{
	int i=0;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
	
	//检查是否有参数改变
	for( i =0;i<m_parameter.dcount;i++)
	{
		if( m_parameter.depots[i] != NULL  &&  m_parameter.depots[i]->modified )
			break;
	}
	
	//如果有参数改变，释放信号量，激活参数保存线程
	if( m_parameter.signal && i< m_parameter.dcount)
	{
		m_parameter.saveall = true;
		sw_signal_give(m_parameter.signal);
	}
	
	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	return true;
}


/**
 * @brief 设置参数值改变的回调函数
 */
void sw_parameter_set_observer(on_para_modified on_modified,void* handle)
{
	if( m_parameter.ocount < MAX_OBSERVER_NUM )
	{
		m_parameter.observers[m_parameter.ocount].on_modified = on_modified;
		m_parameter.observers[m_parameter.ocount].handle = handle;
		m_parameter.ocount++;
	}
}


/** 
 * @brief 读取文本区域的参数
 * 
 * @param name 
 * @param value 
 * @param size 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_get( char* name, char* value, int size )
{
	int i;
	if ( name == NULL || m_parameter.mem == NULL)
		return false;

	if( m_parameter.mutex )
		sw_mutex_lock( m_parameter.mutex );
	
	/* 寻找参数在数组中的位置 */
	i = sw_parameter_find( name );
	if( i < 0 )
	{
		/* 默认的如果是JAVA数据库的话,没有的话直接从java中读取 */
		if ( m_parameter.defdepot->type == IDEPOT_JAVA && m_parameter.pnum < MAX_PARA_NUM)
		{
			i = m_parameter.pnum;
			m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
			m_parameter.plist[i].value = NULL;
			m_parameter.plist[i].depot = m_parameter.defdepot;
			m_parameter.plist[i].readonly = false;
			m_parameter.plist[i].realtime = true;
			m_parameter.pnum++;
			insert_qsort();
			i = sw_parameter_find( name );
			goto PARAMETER_GET;
		}
		if ( value != NULL && 0 < size )
			memset(value,0,size);
		if( m_parameter.mutex )
			sw_mutex_unlock( m_parameter.mutex );
		return false;
	}
PARAMETER_GET:
	/* 对于 JAVA 实时非只读参数,每次都从java读取 */
	if( m_parameter.plist[i].depot->type == IDEPOT_JAVA  
		&& m_parameter.plist[i].realtime == true
		&& m_parameter.plist[i].readonly == false )
	{
		if (m_parameter.plist[i].value != NULL)
		{
			sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
			m_parameter.plist[i].value = NULL;
		}
		if( m_parameter.mutex )
			sw_mutex_unlock( m_parameter.mutex );
		if ( m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,name,value,size) )
			return true;
		return false;
	}
	/* 如果参数表中的参数值为空，从仓库中获取一下*/
	if( m_parameter.plist[i].value == NULL )
	{	/* 但是如果上层只是判断参数是否存在的话此处会造成后续获取参数报错 */
		if ( value != NULL && 0 < size)
			memset(value, 0, size);
		if ( size < 1024 || value == NULL )
		{
			char tmpbuf[1024];
			if ( m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,name,tmpbuf,sizeof(tmpbuf)) )
			{
				if (m_parameter.plist[i].readonly == false || tmpbuf[0] != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem,tmpbuf, __FILE__, __LINE__ );
				if ( value != NULL && 0 < size)
					strlcpy(value, tmpbuf, size);
			}	
		}
		else
		{
			if ( m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,name,value,size) )
			{
				if (m_parameter.plist[i].readonly == false || value[0] != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
	}
	else
	{
		/* 是否需要先memset??由于参数是字串,上层在使用是也要以字串的方式使用,所以无须memset */
		if ( value != NULL && 0 < size )
			strlcpy( value, m_parameter.plist[i].value, size);
	}
   
	if( m_parameter.mutex )
		sw_mutex_unlock( m_parameter.mutex );
		
	return true;
}

/** 
 * @brief 设置文本区域的参数
 * 
 * @param name 
 * @param value 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set( char* name, char* value )
{
	if ( m_parameter.mem == NULL )
		return false;
	return sw_parameter_set_with_depot(name,value,NULL,false,false);
}
	
/*设置参数到指定的depot*/
static bool sw_parameter_set_with_depot(char* name,char* value,sw_idepot_t* depot,bool is_load,bool only_default)
{
	int i,len1,len2;
	
	if( m_parameter.mutex )
		sw_mutex_lock( m_parameter.mutex );
	
	i = sw_parameter_find(name);
	
	if( i >= 0 )
	{
		if( value == NULL )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//如果被写保护了,返回false,JAVA只读参数不允许在此修改,非java参数不为空时不允许修改
		if( m_parameter.plist[i].readonly && (m_parameter.plist[i].depot->type == IDEPOT_JAVA || m_parameter.plist[i].value != NULL))
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//如果只是设置缺省参数，且depot没有改变，则直接返回
		if( only_default && (depot == m_parameter.plist[i].depot || depot == NULL) )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//如果指定的参数仓库和本身的参数仓库有区别，且级别没有提高(type越大级别月底）
		//则直接返回
		if( depot != NULL && depot !=  m_parameter.plist[i].depot 
			&& depot->type >= m_parameter.plist[i].depot->type ) 
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}
		/* JAVA实时参数直接保存到java数据库中,不本地备份 */
		if (false == is_load && m_parameter.plist[i].depot->type == IDEPOT_JAVA && m_parameter.plist[i].realtime)
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			m_parameter.plist[i].depot->set(m_parameter.plist[i].depot,name,value);
			return true;
	  }
		//参数值相同，且没有改变参数仓库，则直接返回true
		if(m_parameter.plist[i].value && strcmp(m_parameter.plist[i].value, value) == 0
			&& (depot == NULL || depot == m_parameter.plist[i].depot) )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);

			return true;
		}

		if( m_parameter.plist[i].value == NULL )
		{
			m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
		}
		else if( strcmp(m_parameter.plist[i].value, value) != 0 )
		{
			len1 = strlen( m_parameter.plist[i].value );
			len2 = strlen( value );
			if( len2 <= len1 )
				strcpy( m_parameter.plist[i].value, value );//有判断空间大小
			else
			{
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
		//变更参数仓库
		if( depot != NULL )
			m_parameter.plist[i].depot = depot;
		
		if( !is_load )
		{
			m_parameter.plist[i].depot->set(m_parameter.plist[i].depot,name,value);
			m_parameter.plist[i].depot->modified = true;
			if( m_parameter.plist[i].depot->autosave)
			{
				m_parameter.saveauto = true;
				sw_signal_give(m_parameter.signal);
			}
		}
		else /**/
		{
			if (m_parameter.plist[i].depot->type == IDEPOT_JAVA)
				m_parameter.plist[i].realtime = true;
		}

		if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
		
		if( !is_load )
		{
			for( i =0;i<m_parameter.ocount; i++ )
			{
				if(m_parameter.observers[i].handle != NULL )
					m_parameter.observers[i].on_modified(name,value,m_parameter.observers[i].handle);
			}
		}

		return true;
	}
	else if( m_parameter.pnum < MAX_PARA_NUM && strlen(name) < 256)
	{//如果参数名太长认为是被攻击了
		i = m_parameter.pnum;
		m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
		if( value )
			m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
		else
			m_parameter.plist[i].value = NULL;
		if( depot == NULL )
			m_parameter.plist[i].depot = m_parameter.defdepot;
		else
			m_parameter.plist[i].depot = depot;

		if( !is_load )
		{
			m_parameter.plist[i].depot->set(m_parameter.plist[i].depot,name,value);
			m_parameter.plist[i].depot->modified = true;
			if( m_parameter.plist[i].depot->autosave)
			{
				m_parameter.saveauto  =  true;
				sw_signal_give(m_parameter.signal);
			}
		}
		
		if (m_parameter.plist[i].depot->type == IDEPOT_JAVA)
			m_parameter.plist[i].realtime = true;
		m_parameter.plist[i].readonly = false;
		m_parameter.pnum ++;
		/* 进行排序, 加入时就排序,逐渐加入 */
		insert_qsort();
		if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);

		return true;
	}

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	return false;

}

/* 添加一组默认的参数,如果only_default为true的话需要更新老的参数值,如果为false的话存在就不用更新了 */
static bool group_parameter_update(char *buf, int size, bool only_default)
{
	char *p, *e, *name_s, *name_e;
	char *name, *value;
	int strsize, len, num = 0;

	strsize = strlen( buf );
	if( size < strsize )
		strsize = size;

	/* 分析数据 */
	p = buf;
	while( p < buf + strsize )
	{
		/* 去掉开始的空格 */
		while( *p == ' ' ||  *p == '\t' ||  *p == '\r' ||  *p == '\n' )
			p++;
		name_s = p;

		/* 找行结束 */
		while( *p != '\r' && *p != '\n' && *p != '\0' )
			p++;
		e = p;

		/* 忽略: 注释,空行,老版本参数信息 */
		if( *name_s == '#' || e <= name_s )
			goto NEXT_ROW;

		/* 找到name/value分隔符，得到参数名称 */
		p = name_s;
		while( *p != ' ' && *p != '\t' && *p != ':' && *p != '=' && p < e )
			p++;
		if( p <= name_s )
			goto NEXT_ROW;
		name_e = p;

		/* 跳过value之前的分隔符，得到参数值 */
		p++;
		while( *p == ' ' || *p == '\t' || *p == ':' || *p == '=' )
			p++;

		if( p <= e )
		{
			len = name_e - name_s;
			name = sw_mem_alloc( m_parameter.mem, len+1, __FILE__, __LINE__ );
			if( name )
			{
				memmove( name, name_s, len );
				name[len] = 0;
			}
			else
				break;
			len = e - p;
			value = sw_mem_alloc( m_parameter.mem, len+1, __FILE__, __LINE__ );
			if( value )
			{
				memmove( value, p, len );
				value[len] = 0;
			}
			else
				break;
			if ( !only_default )
			{/* 需要刷新老的参数值 */
				sw_parameter_set( name, value );
			}
			else
			{/* 如果不存在需要添加到默认的参数仓库中 */
				sw_parameter_set_with_depot(name, value, m_parameter.defdepot, true, true);
			}
			sw_mem_free( m_parameter.mem, name, __FILE__, __LINE__ );
			sw_mem_free( m_parameter.mem, value, __FILE__, __LINE__ );
			num++;
		}
NEXT_ROW:
		p = e + 1;
	}

	return 0 < num;
}
/** 
 * @brief 设置一组参数
 * 
 * @param buf 
 * @param size 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set_group( char *buf, int size )
{
	return group_parameter_update(buf, size, false);
}

/**
 * @brief 从内存中添加一组默认的参数到默认的参数仓库中，如果参数存在则保留员参数,参数不存在则添加到默认的仓库中
 *
 * @param buf,参数数组
 * @param size,数组大小
 *
 * return true,更新成功; false,更新失败
 */
bool sw_parameter_set_group_default(char *buf, int size)
{
	return group_parameter_update(buf, size, true);
}

/**
 * @brief 强制修改对应的参数仓库位置,并设置其只读属性,java参数的实时读取属性,如果是java参数的话需要从APK数据库更新参数值,
 					如果参数库位置更改,原始有值的话不修改参数值，否则设为默认值,
 					此接口只允许在app初始化时调用
 *
 * @param name,参数名
 * @param defaultvalue,参数默认值
 * @param readonly,参数只读
 * @param realtime,实时读取
 * @param depot,实时读取
 *
 * return true,参数库位置改变后返回true
 */
bool sw_parameter_update_with_depot(char *name, char *defaultvalue, bool readonly, bool realtime, sw_idepot_t* depot)
{
	if (name == NULL || *name == '\0' || defaultvalue == NULL || depot == NULL)
		return false;
	sw_mutex_lock( m_parameter.mutex );

	int i = sw_parameter_find(name);
	if (i >= 0)
	{
		m_parameter.plist[i].readonly = readonly;
		m_parameter.plist[i].realtime = realtime;
		if (depot == m_parameter.plist[i].depot)
		{
			if (depot->type == IDEPOT_FILE && 
					(m_parameter.plist[i].value == NULL || m_parameter.plist[i].value[0] == '\0') )
			{
				if (m_parameter.plist[i].value)
					sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = NULL;
				if (*defaultvalue != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
			}
			sw_mutex_unlock( m_parameter.mutex );
			return false;
		}
		if (depot->type == IDEPOT_JAVA)
		{
			char value[1024];
			depot->get(depot,name,value,sizeof(value));
			if (value[0] == '\0')
			{
				if (readonly == false && m_parameter.plist[i].value && m_parameter.plist[i].value[0] != '\0')
					depot->set(depot, name, m_parameter.plist[i].value);
			}
			else
			{
				if (m_parameter.plist[i].value)
					sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
		else if (depot->type == IDEPOT_FILE)
		{
			if (m_parameter.plist[i].value == NULL || m_parameter.plist[i].value[0] == '\0')
			{
				if (m_parameter.plist[i].value)
					sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
				m_parameter.plist[i].value = NULL;
				if (*defaultvalue != '\0')
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
			}
		}
		m_parameter.plist[i].depot = depot;
		sw_mutex_unlock( m_parameter.mutex );
		return true;
	}
	else if( m_parameter.pnum < MAX_PARA_NUM && strlen(name) < 256)
	{
		i = m_parameter.pnum;
		m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
		m_parameter.plist[i].readonly = readonly;
		m_parameter.plist[i].realtime = realtime;
		m_parameter.plist[i].value = NULL;
		m_parameter.plist[i].depot = depot;
		if (depot->type == IDEPOT_JAVA)
		{
			char value[1024];
			depot->get(depot,name,value,sizeof(value));
			if (value[0] == '\0')
			{
				if (readonly == false && defaultvalue != '\0')
				{
					depot->set(depot, name, defaultvalue);
					m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
				}
			}
			else
			{
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, value, __FILE__, __LINE__ );
			}
		}
		else if (*defaultvalue != '\0')
		{
			if (depot->type == IDEPOT_FILE)
				m_parameter.plist[i].value = sw_mem_strdup( m_parameter.mem, defaultvalue, __FILE__, __LINE__ );
		}
		m_parameter.pnum ++;
		/* 进行排序, 加入时就排序,逐渐加入 */
		insert_qsort();
		sw_mutex_unlock(m_parameter.mutex);
		return false;
	}
	sw_mutex_unlock(m_parameter.mutex);
	return false;
}

/**
 * @brief 刷新java参数库中的只读和非实时读取的参数
 */
void sw_parameter_refresh(void)
{
	sw_mutex_lock( m_parameter.mutex );
	int i = 0;
	for (i = 0; i < m_parameter.pnum; i++)
	{
		if (m_parameter.plist[i].depot->type == IDEPOT_JAVA 
				&& (m_parameter.plist[i].readonly || m_parameter.plist[i].realtime == false))
		{
			char value[1024];
			m_parameter.plist[i].depot->get(m_parameter.plist[i].depot,m_parameter.plist[i].name,value,sizeof(value));
			if (value[0] == '\0')
			{
				if (m_parameter.plist[i].value)
					sw_mem_free(m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__);
				m_parameter.plist[i].value = NULL;
			}
			else
			{
				if (m_parameter.plist[i].value == NULL)
					m_parameter.plist[i].value = sw_mem_strdup(m_parameter.mem, value, __FILE__, __LINE__);
				else if (strcmp(value, m_parameter.plist[i].value) != 0)
				{
					sw_mem_free(m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__);
					m_parameter.plist[i].value = sw_mem_strdup(m_parameter.mem, value, __FILE__, __LINE__);
				}
			}
		}
	}
	sw_mutex_unlock(m_parameter.mutex);
}

/** 
 * @brief 读取文本区域的参数,返回参数的数值
 * 
 * @param name 
 * 
 * @return 参数的数值
 */
int sw_parameter_get_int( char* name )
{
	char text[16];

	if( sw_parameter_get( name, text, sizeof( text ) ) )
		return atoi( text );
	return -1;

}

/** 
 * @brief 按数值设置文本区域的参数
 * 
 * @param name 
 * @param value 参数的数值
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set_int( char* name, int value )
{
	char text[16];
	sprintf( text, "%d", value );
	return sw_parameter_set( name, text );
}

/** 
 * @brief 设置参数的缺省值,当参数不存在则加到参数表中，如果参数存在，且参数仓库没有改变则不做任何处理
 *	如果参数仓库级别更高，则更改参数仓库	
 * 
 * @param name 参数名称
 * @param value 参数值
 * @param depot 指定的参数仓库
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_set_default( char* name, char* value,sw_idepot_t* depot)
{
	return sw_parameter_set_with_depot(name,value,depot,true,true);
}


/** 
 * @brief 增加参数到指定的参数仓库
 * 
 * @param name 参数名称
 * @param value 参数值
 * @param depotname 指定的参数仓库
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_add( char* name, char* value,char* depotname )
{
	int i;
	sw_idepot_t* depot = NULL;
	
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	i = depot_find_byname(depotname);

	if( i>=0 && i< m_parameter.dcount )
		depot = m_parameter.depots[i];

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	if( depot )
		sw_parameter_set_with_depot(name,value,depot,false,false);
	
	return (depot != NULL );
}

/** 
 * @brief 删除文本区域的参数
 * 
 * @param name 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_delete( char* name )
{
	int i;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* 寻找参数在数组中的位置 */
	i = sw_parameter_find( name );
	if( 0 <= i )
	{
		//如果被写保护了,返回false
		if( m_parameter.plist[i].readonly )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		//如果对于 java 参数，不能删除
		if( m_parameter.plist[i].depot->type == IDEPOT_JAVA )
		{
			if(m_parameter.mutex)
				sw_mutex_unlock(m_parameter.mutex);
			return false;
		}

		if( m_parameter.plist[i].name )
			sw_mem_free( m_parameter.mem, m_parameter.plist[i].name, __FILE__, __LINE__ );
		if( m_parameter.plist[i].value )
			sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
	
		/*设置参数有改变*/
		m_parameter.plist[i].depot->modified = true;

		/* 后面的前移 */
		if( i+1 < m_parameter.pnum )
			memmove( m_parameter.plist+i, m_parameter.plist+i+1, sizeof(para_desc_t)*(m_parameter.pnum-i-1) );
		m_parameter.pnum--;
		
		if(m_parameter.mutex)
			sw_mutex_unlock(m_parameter.mutex);
		
		return true;
	}
	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
		
	return false;

}


/** 
 * @brief 清除缺省仓库中的所有参数
 */
void sw_parameter_delete_all()
{
	int i,j,maxnum;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
	maxnum = m_parameter.pnum;
	for( i=0, j=0; i< maxnum; i++ )
	{
		if( m_parameter.plist[i].readonly ||  m_parameter.plist[i].depot !=  m_parameter.defdepot )
		{
			if ( j != i )
			{
				memmove( &m_parameter.plist[j], &m_parameter.plist[i], sizeof(para_desc_t));
				memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
			}
			j++;
		}
		else
		{
			if( m_parameter.plist[i].name )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].name, __FILE__, __LINE__ );
			if( m_parameter.plist[i].value )
				sw_mem_free( m_parameter.mem, m_parameter.plist[i].value, __FILE__, __LINE__ );
	
			memset( &m_parameter.plist[i], 0, sizeof(para_desc_t));
		}
	}
	m_parameter.pnum = j;
	m_parameter.defdepot->modified  = true;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
}

/** 
 * @brief 取得第一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_first( unsigned long *pos )
{
	if( 0 < m_parameter.pnum )
	{
		if( pos )
			*pos = 0;
		return m_parameter.plist[0].name;
	}
	return NULL;

}

/** 
 * @brief 取得下一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_next( unsigned long *pos )
{
	if( ((int)*pos)+1 < m_parameter.pnum )
	{
		*pos = (*pos) + 1;
		return m_parameter.plist[*pos].name;
	}
	else
		return NULL;

}

/** 
 * @brief 取得上一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_prev( unsigned long *pos )
{
	if( 0 < *pos )
	{
		*pos = (*pos) - 1;
		return m_parameter.plist[*pos].name;
	}
	else
		return NULL;
}

/** 
 * @brief 取得最后一个参数,返回NULL表示没有找到
 * 
 * @param pos 
 * 
 * @return 
 */
char* sw_parameter_get_last( unsigned long *pos )
{

	if( 0 < m_parameter.pnum )
	{
		*pos = m_parameter.pnum - 1;
		return m_parameter.plist[*pos].name;
	}
	else
		return NULL;

}

/** 
 * @brief 取得参数的POS参数,返回NULL表示没有找到
 * 
 * @param name 
 * @param pos 
 * 
 * @return true,成功; false, 失败
 */
bool sw_parameter_get_pos( char *name, unsigned long *pos )
{
	int i = sw_parameter_find( name );
	if( i < 0 )
		return false;
	else if( pos )
		*pos = i;
	return true;
}

/** 
 * @brief 根据POS读取参数,返回NULL表示没有找到
 * 
 * @param pos 
 * @param *value 
 * 
 * @return 
 */
char* sw_parameter_get_by_pos( unsigned long pos, char **value )
{
	char* name = NULL;
	if( (int)pos >= m_parameter.pnum || value == NULL )
		return NULL;

	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);

	/* 对于 JAVA 参数， 每次都从java读取 */
	if( m_parameter.plist[pos].depot->type == IDEPOT_JAVA  
		&& m_parameter.plist[pos].realtime
		&& m_parameter.plist[pos].readonly == false
		)
	{
		if (m_parameter.plist[pos].value != NULL)
		{
			sw_mem_free( m_parameter.mem, m_parameter.plist[pos].value, __FILE__, __LINE__ );
			m_parameter.plist[pos].value = NULL;
		}
	}

	if( m_parameter.plist[pos].value == NULL )
	{
		char temp[1024];
		memset(temp,0,sizeof(temp));
		m_parameter.plist[pos].depot->get(m_parameter.plist[pos].depot,m_parameter.plist[pos].name,temp,sizeof(temp)-1);
		m_parameter.plist[pos].value = sw_mem_strdup( m_parameter.mem, temp, __FILE__, __LINE__ );
	}

	if( m_parameter.plist[pos].value )
	{
		*value = m_parameter.plist[pos].value;
		name = m_parameter.plist[pos].name;
	}

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);

	return name;
}

/** 
 * @brief 取得参数个数
 * 
 * @return 
 */
int sw_parameter_get_num()
{
	return m_parameter.pnum;
}


/**
 * @brief 设置某个函数写保护
 *
 * @param name 要保护的参数名程
 *
 * return 成功返回true，失败返回false
 */
bool sw_parameter_set_readonly(char* name,bool readonly)
{
	if ( name == NULL )
		return false;
	int i;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* 寻找参数在数组中的位置 */
	i = sw_parameter_find( name );
	if( i >= 0 )
		m_parameter.plist[i].readonly  = readonly;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return (i >=0 );
}	

/**
 * @brief 去掉某个参数的写保护
 *
 * @param name 要写保护的参数名称
 *
 * return 成功返回true，失败返回false
 */
bool sw_parameter_get_readonly(char* name)
{
	if ( name == NULL )
		return false;
	int i;
	bool readonly =false;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* 寻找参数在数组中的位置 */
	i = sw_parameter_find( name );
	if( i >= 0 )
		readonly = m_parameter.plist[i].readonly;

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return readonly;
}

/**
 * @brief 设置参数是否实时读取:目前只对java有用,默认的java数据库参数都是实时读取的
 *
 * @param name 要实时读写的参数名程
 *
 * return 成功返回true，失败返回false
 */
bool sw_parameter_set_realtime(char* name,bool realtime)
{
	if ( name == NULL )
		return false;
	int i;
	if(m_parameter.mutex)
		sw_mutex_lock(m_parameter.mutex);
		
	/* 寻找参数在数组中的位置 */
	i = sw_parameter_find( name );
	if( i >= 0 )
	{
		m_parameter.plist[i].realtime  = realtime;
	}
	else if (m_parameter.pnum < MAX_PARA_NUM && m_parameter.defdepot->type == IDEPOT_JAVA)
	{
			i = m_parameter.pnum;
			m_parameter.plist[i].name = sw_mem_strdup( m_parameter.mem, name, __FILE__, __LINE__ );
			m_parameter.plist[i].value = NULL;
			m_parameter.plist[i].depot = m_parameter.defdepot;
			m_parameter.plist[i].readonly = false;
			m_parameter.plist[i].realtime = realtime;
			m_parameter.pnum ++;
			insert_qsort();
	}
	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	return (i >=0 );
}	

/* 查找参数所在的序号,由于加入的参数已经排好序了，使用折半查找 */
static int sw_parameter_find( char *name )
{
	if(name == NULL )
		return -1;
#if 1
	int big = 0, end = m_parameter.pnum-1;
	int i, mid;
	if ( m_parameter.pnum == 0 )
		return -1;
	//升序
	while ( big <= end )
	{
		mid = (big+end)>>1;
		i=strcasecmp(name, m_parameter.plist[mid].name);
		if ( 0 < i )
		{//mid比name小,升序时,更改起始位置
			big = mid+1;
		}
		else if ( i < 0 )
		{//更改结束位置
			end = mid-1;
		}
		else
			return mid;
	}
#else
	int i = 0;
	int maxnum = m_parameter.pnum;
	for ( ; i < maxnum ; i++)
	{
		if ( strcasecmp(m_parameter.plist[i].name, name) == 0 )
			return i;
	}
#endif
	return -1;
}

/* 插入一个参数后调用的排序接口,插入参数放到最后了 */
static void insert_qsort(void)
{/**/
	if ( m_parameter.pnum <= 1 )
		return ;
	int big = 0, end = m_parameter.pnum-2;
	int i, mid;
	char *name = m_parameter.plist[end+1].name;
	para_desc_t tmp;
	memcpy(&tmp, m_parameter.plist+m_parameter.pnum-1, sizeof(para_desc_t));
	/* 快速查找到参数位置 */
	while ( big <= end )
	{
		mid = (big+end)/2;
		i = strcasecmp(name, m_parameter.plist[mid].name);
		if ( 0 < i )
		{//mid比name小,升序时,更改起始位置
			big = mid + 1;
		}
		else if ( i < 0 )
		{
			end = mid - 1;
		}
		else
		{
			return;
		}
	}

	if ( 0 < i )
	{//mid比name小,升序时,从mid往后查找比name大的位置
		int maxpos = m_parameter.pnum - 1;
		while ( mid < maxpos )
		{
			if ( 0 < strcasecmp(name, m_parameter.plist[mid].name) )
			{
				mid++;
			}
			else
			{
				memmove(m_parameter.plist+mid+1, m_parameter.plist+mid, sizeof(para_desc_t)*(m_parameter.pnum-1-mid));
				memcpy(m_parameter.plist+mid, &tmp, sizeof(para_desc_t));
				return;	
			}
		}
	}
	else
	{//mid比name大往前找找到最好一个比name大的位置
		while ( 0 <= mid )
		{
			if ( strcasecmp(name, m_parameter.plist[mid].name) < 0 )
				mid--;
			else
			{
				mid++;
				break;
			}
		}
		if ( mid < 0 )
			mid = 0;
		memmove(m_parameter.plist+mid+1, m_parameter.plist+mid, sizeof(para_desc_t)*(m_parameter.pnum-1-mid));
		memcpy(m_parameter.plist+mid, &tmp, sizeof(para_desc_t));
	}
}

/*通过仓库名查找参数仓库*/
static int depot_find_byname(char* name)
{
	int i;
	
	if(name == NULL )
		return -1;

	for( i=0; i<m_parameter.dcount; i++ )
	{
		if( strcasecmp(m_parameter.depots[i]->name,name) == 0 )
			return i;
	}
	return -1;
}


//参数仓库收集参数的回调函数,返回-1表示没有后续的参数
static int on_gather_para(sw_idepot_t *depot,int pos,char** name,char** value)
{	
	int i;
	
	if( pos < 0 )
		pos = 0;
	
	for( i=pos; i<m_parameter.pnum; i++)
	{
		if( m_parameter.plist[i].depot == depot )
		{
			*name =  m_parameter.plist[i].name;
			*value  =  m_parameter.plist[i].value;
			break;
		}
	}
	
	if( i <  m_parameter.pnum )	
		return (i+1);
	else
		return -1;
}

//参数保存线程
static bool param_save_proc(uint32_t wparam,uint32_t lparam)
{
	int i;
	sw_parameter_t* parameter = (sw_parameter_t*)wparam;

	sw_signal_wait( parameter->signal,-1);
	
	//如果在超时之前，有新参数要保存，则继续等待
	while( !parameter->exit  && (sw_signal_wait(parameter->signal,parameter->timeout)==0) );

	//超时了，保存参数吧
	if(parameter->mutex)
		sw_mutex_lock(parameter->mutex);
	
	//检查是否有参数改变
	if( parameter->saveall)
	{
		for( i =0;i<parameter->dcount;i++)
		{
			if( parameter->depots[i] != NULL  &&  parameter->depots[i]->modified )
			{
				parameter->depots[i]->modified = false;
			 	parameter->depots[i]->save(parameter->depots[i],on_gather_para);
			}	
		}
 		parameter->saveall = false;
		parameter->saveauto =false;
	}
	else if( parameter->saveauto)
	{
		for( i =0;i<parameter->dcount;i++)
		{
			if( parameter->depots[i] != NULL  &&  parameter->depots[i]->modified &&  parameter->depots[i]->autosave )
			{
				parameter->depots[i]->modified = false;
			 	parameter->depots[i]->save(parameter->depots[i],on_gather_para);
			}	
		}
 		parameter->saveauto = false;
	}	

	if(m_parameter.mutex)
		sw_mutex_unlock(m_parameter.mutex);
	
	//判断是否退出
	if( parameter->exit )
	{
		parameter->thrd = NULL;
		return false;
	}
	return true;
}
