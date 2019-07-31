/** 
 * @file swtimer.c
 * @brief 定义定时器函数接口,注意在timer的回调函数里只适合处理耗时比较小的事情，
 *		否则会影响整个定时器系统的准确性
 *
 * @author hujinshui/chenkai
 * @date 2010-07-6 created
 */
  
#include "swapi.h"
#include "swutil_priv.h"
#include "swthrd.h"
#include "swmutex.h"
#include "swmem.h"
#include "swtimer.h"
#include "swsignal.h"

#define MAX_TIMERNAME_NUM	64
#define MAX_MEM_SIZE      64*1024

#define timer_alloc( size ) sw_mem_alloc( m_timer_des.hmem ,size, __FILE__, __LINE__ )
#define timer_free( p ) sw_mem_free( m_timer_des.hmem , p ,__FILE__, __LINE__ )

/**   
 * @brief  定时器数据结构
 * 
 * @param name[MAX_TIMERNAME_NUM]				定时器名字
 * @param interval		定时器触发的间隔时间
 * @param times				定时器运行事件的次数
 * @param handler			定时器处理函数
 * @param wparam			第一个参数
 * @param wparam			第二个参数
 * @param type				定时器类型
 * @param status			定时器状态
 * 
 */
typedef struct timer
{
	char name[MAX_TIMERNAME_NUM];
	HANDLE handle; 					//timernode的句柄
	unsigned long interval; 		//设置的值,即该任务间隔多长时间处理一次事件
	long times;    		//该事件执行多少次
	long original_times; 	//记录原始的times
	unsigned long start_time;
	unsigned long trigger_time;  	//事件处理的触发时间  
	unsigned long store_time;   	//记录上次运行剩下的时间
	ontimehandler_t handler;    	//处理函数
	uint32_t wparam;				//处理函数的第一个参数
	uint32_t lparam;				//处理函数的第二个参数
	timer_status_t  status;			//定时器的状态
	struct timer *next;				
	struct timer *prev;				

}timer_nod_t;

typedef struct timer_des
{
	HANDLE 			htimer_thrd;
	HANDLE 			htimer_mutex;
	HANDLE			htimer_signal;
	HANDLE 			hmem;
	timer_nod_t *ptimer_head;
	timer_nod_t *ptimer_tail;
	char		*ptimer_buf;
	unsigned long timeout;    
	bool	exit;
}timer_des_t;

static timer_des_t m_timer_des;    //全局timer描述结构


/**
 * @brief 比较timeout的值,取当前链表中最小的，并更新
 */
static void upgrade_timeout( timer_nod_t *ptimer )
{
	if( NULL == ptimer )
	{
		return ;
	}

	int now;
	now = sw_thrd_get_tick( );
	if( m_timer_des.timeout > ( ptimer->trigger_time - now ) )
	{
		m_timer_des.timeout = ptimer->trigger_time - now;	
	}
}

/**
 * @brief 	添加一个定时器结点,使它入定时器链表
 *
 * @param	ptimer 定时器结点指针 	
 *
 * @return 0表示定时器结点成功加入了链表， -1表示加入失败
 */
static int add_timer( timer_nod_t *ptimer )
{
//	timer_nod_t *tmp_timer;

	if ( NULL == ptimer )
	{
		return -1;
	}

	sw_mutex_lock(m_timer_des.htimer_mutex); 

	upgrade_timeout( ptimer );


	if ( m_timer_des.ptimer_head == NULL && m_timer_des.ptimer_tail == NULL )   //第一次插结点入队列
	{
		ptimer->prev = NULL;
		ptimer->next = NULL;
		m_timer_des.ptimer_head = ptimer;
		m_timer_des.ptimer_tail = ptimer;

		sw_mutex_unlock( m_timer_des.htimer_mutex );

		sw_signal_give( m_timer_des.htimer_signal );

		return 0;
	}
	else	 //在表尾插入
	{
		ptimer->prev = m_timer_des.ptimer_tail;
		ptimer->next = NULL;
		m_timer_des.ptimer_tail->next = ptimer;
		m_timer_des.ptimer_tail = ptimer;

		sw_mutex_unlock( m_timer_des.htimer_mutex );

		sw_signal_give( m_timer_des.htimer_signal );

		return 0;
	}

	return -1;		
}




/**
 * @brief 从链表中删除一个定时器结点
 * param  定时器结点
 *
 * @return 0表示删除成功  -1表示删除失败
 */
static int delete_timer( timer_nod_t *ptimer )
{
	if ( NULL == ptimer )
	{	
		return -1;
	}

	if ( m_timer_des.ptimer_head == NULL )   //队列队列为空，返回错误
	{
		UTIL_LOG_INFO( "删除一个空队列!" );

		return -1;
	}
	else
	{
		//删除结点，此时链表中只有一个结点
		if( ptimer == m_timer_des.ptimer_head && ptimer == m_timer_des.ptimer_tail  )
		{
			m_timer_des.ptimer_head = NULL;
			m_timer_des.ptimer_tail = NULL;
			timer_free( ptimer );   	
		}

		//删除处于表头的结点
		else if( ptimer == m_timer_des.ptimer_head && ptimer != m_timer_des.ptimer_tail  )
		{
			m_timer_des.ptimer_head = ptimer->next;
			ptimer->next->prev = NULL;
			timer_free( ptimer );   	
		}

		//删除处于表尾的结点
		else if( ptimer != m_timer_des.ptimer_head && ptimer == m_timer_des.ptimer_tail )
		{
			m_timer_des.ptimer_tail = ptimer->prev;
			ptimer->prev->next = NULL;
			timer_free( ptimer );
		}

		//删除处于表中的结点
		else if( ptimer != m_timer_des.ptimer_head && ptimer != m_timer_des.ptimer_tail )
		{
			ptimer->prev->next = ptimer->next;
			ptimer->next->prev = ptimer->prev;
			timer_free( ptimer );   
		}

		return 0;
	}

}

/** 
 * @brief 定时时间到了，根据相关条件处理事件
 * 
 * @param ptimer  定时器结点 
 *
 * return  无
 */
static void on_tick( timer_nod_t *ptimer )
{
	unsigned long now;

	if ( NULL == ptimer )
	{
		return;
	}
	if ( ptimer->status != TIMER_STATUS_ENABLE )
	{
		return;
	}
	if ( ptimer->times == 0 )    //表示N次事件处理到此结束
	{
		return;
	}

	if( ptimer->times > 0 && ptimer->status == TIMER_STATUS_ENABLE )
	{
		now = sw_thrd_get_tick();

		if( ptimer->trigger_time <= now )
		{
			ptimer->handler( ptimer->wparam, ptimer->lparam );
			ptimer->times--;
			UTIL_LOG_INFO( "[%s]the rest times is %ld,now is %u \n ",__FUNCTION__,ptimer->times,sw_thrd_get_tick() );

			if( ptimer->times > 0 )   //如果times大于0，表示还有下次触发事件，如果为0表示没有下次触发事件
			{
				ptimer->trigger_time = ptimer->trigger_time + ptimer->interval;
			}
			else if(ptimer->times == 0 ) 
			{
				UTIL_LOG_INFO( "return to the thread_pro \n" );
			
				return ;
			}
			
		}
	}
	else if( ptimer->times == -1 && ptimer->status == TIMER_STATUS_ENABLE )
	{
		now = sw_thrd_get_tick();
		if( ptimer->trigger_time <= now )
		{
			ptimer->handler( ptimer->wparam, ptimer->lparam );
			UTIL_LOG_INFO( "[%s]the rest times is %ld,now is %u \n ",__FUNCTION__,ptimer->times,sw_thrd_get_tick() );

			ptimer->trigger_time = ptimer->trigger_time + ptimer->interval;
		}
	}

}


/** 
 * @brief 查询链表中，最小的timeout值
 *	
 * return 返回当前链表中timeout值最小的结点		
 */
static unsigned long find_min_timeout() //返回一个最小的timeout
{
	timer_nod_t *tmp_timer;
	unsigned long now;
	unsigned long min_timeout;
	min_timeout = m_timer_des.timeout;

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( tmp_timer = m_timer_des.ptimer_head; tmp_timer != NULL ; tmp_timer = tmp_timer->next )		
	{
		if( tmp_timer->status == TIMER_STATUS_ENABLE )
		{
			now = sw_thrd_get_tick( );

			if( min_timeout > ( tmp_timer->trigger_time - now ) )
			{
				min_timeout = tmp_timer->trigger_time - now;

				sw_mutex_unlock( m_timer_des.htimer_mutex );
			}
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return min_timeout;	
}

/** 
 * @brief 主时钟线程
 * 
 * @param wparam  线程的第一个参数
 * @param wparam  线程的第二个参数
 *
 * return 
 */

static bool thread_pro( unsigned long wparam, unsigned long lparam )
{
	timer_des_t *time_des = (timer_des_t*)wparam;
	timer_nod_t *ptimer;

	time_des->timeout = -1;
	time_des->timeout = find_min_timeout(); 		

	sw_signal_wait( time_des->htimer_signal, time_des->timeout );   //信号量，等待时间是timeout
	if (time_des->exit)
	{
		time_des->htimer_thrd = NULL;
		return false;
	}

	sw_mutex_lock( time_des->htimer_mutex );    //给线程加锁，原子操作,准备遍历链表

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )	//遍历链表，到点了就触发事件
	{
		on_tick(ptimer);
	}

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer->times == 0 )
		{
			delete_timer( ptimer );
		}
	}

	sw_mutex_unlock( time_des->htimer_mutex );

	return true ;
}


/** 
 * @brief 释放队列资源
 * 
 * @return 无 
 */
static int release_queue()
{
	timer_nod_t *ptimer = NULL;	
	timer_des_t *time_des = &m_timer_des; 

	sw_mutex_lock( time_des->htimer_mutex );

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )	
	{
		ptimer->status =  TIMER_STATUS_DISABLE;
	}

	for ( ptimer = time_des->ptimer_head; ptimer != NULL; ptimer = ptimer->next )	
	{
		if( -1 == delete_timer( ptimer ) )
		{
			UTIL_LOG_INFO("delete timer failed \n");
			sw_mutex_unlock( time_des->htimer_mutex );

			return -1;
		}
	}

	sw_mutex_unlock( time_des->htimer_mutex );

	return 0;
}

/** 
 * @brief 初始化定时器
 * 
 * @param priority  定时器线程的优先级 
 * @param stacksize 分配给定时器线程堆栈的大小
 *
 * @return 0,初始化成功; 错误号, 失败
 */
int sw_timer_init( int priority, int stacksize )
{
	m_timer_des.ptimer_buf = (char *) malloc( MAX_MEM_SIZE );

	if( NULL == m_timer_des.ptimer_buf )
	{
		UTIL_LOG_ERROR( "[sw_timer_init] no memory %d\n",MAX_MEM_SIZE );

		return -1;
	}

	m_timer_des.hmem = sw_mem_init( m_timer_des.ptimer_buf, MAX_MEM_SIZE, 4 );

	if( NULL == m_timer_des.hmem )
	{
		UTIL_LOG_ERROR( "[sw_timer_init] sw_mem_init error \n" );

		return -1;
	}

	//队列初始化
	m_timer_des.ptimer_head = NULL;
	m_timer_des.ptimer_tail = NULL;
	UTIL_LOG_INFO( "timerinit  : queue is inited!\n" );

	//初始化互斥量
	m_timer_des.htimer_mutex = sw_mutex_create();
	UTIL_LOG_INFO( "timerinit  : mutex is inited!:%p\n", m_timer_des.htimer_mutex );

	//初始化信号量
	m_timer_des.htimer_signal = sw_signal_create( 0, 1 );
	UTIL_LOG_INFO( "timerinit  : signal is inited\n" );

	//设置初始等待时间为-1
	m_timer_des.timeout = -1;
	m_timer_des.exit = false;

	//起线程thread_pro
	m_timer_des.htimer_thrd = sw_thrd_open( "swtimer", priority, 0, stacksize, 
								(threadhandler_t)thread_pro, (uint32_t)(&m_timer_des), 0 );
	UTIL_LOG_INFO( "timerinit  :  thread is inited!\n" );

	if( m_timer_des.htimer_thrd )
	{	
		sw_thrd_resume( m_timer_des.htimer_thrd );
	}
	UTIL_LOG_INFO( "timerinit  :  thread is resumed!\n" );

	return 0;	

}


/**
 * @brief 退出timer 模块，释放相应的资源
 * 
 * @return 无
 */
void sw_timer_exit( )
{
	//释放队列所占用的资源
	if( -1 == release_queue() )
	{
		UTIL_LOG_ERROR("queue release failed\n");
	}
	m_timer_des.exit = true;
	sw_signal_give( m_timer_des.htimer_signal );
	//关闭线程
	if( m_timer_des.htimer_thrd )
	{
		sw_thrd_close( m_timer_des.htimer_thrd , 2000 );
		m_timer_des.htimer_thrd = NULL;
	}

	//删除信号量
	if( m_timer_des.htimer_signal )
	{	
		sw_signal_destroy( m_timer_des.htimer_signal );
		m_timer_des.htimer_signal = NULL;
	}

	//删除互斥量
	if( m_timer_des.htimer_mutex )
	{
		sw_mutex_destroy( m_timer_des.htimer_mutex );
		m_timer_des.htimer_mutex = NULL;
	}

	//最后删除定时器模块的资源
	if( m_timer_des.hmem )
	{
		sw_mem_exit( m_timer_des.hmem );
		m_timer_des.hmem = NULL;
	}
}

/**
 * @brief 创建一个定时器
 *
 * @param name,定时器的名称
 * @param interval, 定时器时间间隔
 * @param times,定时器重复次数，0表示无限次
 * @param handler,定时器回调函数
 * @param wparam,定时器回调参数1
 * @param lparam,定时器回调参数2
 *
 * @return 句柄 ; NULL,失败
 */
HANDLE sw_timer_create( char* name,int interval,int times,
		ontimehandler_t	handler,uint32_t wparam,uint32_t lparam )     //相当于一个newtimer ,创建一个node,并且给node赋值
{
	timer_nod_t *ptimer;
	if( NULL == name )
	{
		UTIL_LOG_ERROR( "param interval is invalid value" );

		return NULL;
	}
	//判断interval和times输入的合法性
	if( interval < 0 )
	{
		UTIL_LOG_ERROR( "param interval is invalid value" );

		return NULL;
	}	
	if( times < -1 )
	{
		UTIL_LOG_ERROR( "param times is invalid value" );

		return NULL;
	}

	//判断handler输入是否合法
	if( NULL == (ontimehandler_t)handler )
	{
		UTIL_LOG_ERROR( "param handler is invalid value(NULL)" );

		return NULL;
	}
    
	//判断ptimer是否申请成功
	ptimer = ( timer_nod_t* )timer_alloc( sizeof( timer_nod_t ) );
	if ( ptimer == NULL )
	{
		UTIL_LOG_ERROR("timercreate  :  alloc memory failed \n ");

		return NULL;   
	}
	
	UTIL_LOG_INFO("[%s] : has alloc memory for %s \n",__FUNCTION__, name );

	//对创建的timer_node 初始化赋值；
	strlcpy( ptimer->name, name, sizeof( ptimer->name ) );
	ptimer->status = TIMER_STATUS_ENABLE; //控制
	ptimer->interval = interval; //时钟计数	
	ptimer->times = times;
	ptimer->original_times = times;
	ptimer->handler = handler;
	ptimer->start_time = sw_thrd_get_tick();
	ptimer->trigger_time = ptimer->start_time + ptimer->interval;

	if ( add_timer( ptimer ) != 0 )  //把add_timer()也在sw_timer_create()中实现
	{
		UTIL_LOG_ERROR("timercreate  :   timer node add to the list is failed\n");
		timer_free( ptimer );

		return NULL;
	}

	UTIL_LOG_INFO("[%s]  :%s has add to the list \n ",__FUNCTION__,name );

	return (HANDLE)ptimer ;
}

/**
 * @brief 销毁指定定时器
 * @param handle,定时器句柄
 *
 * @return 无
 */
void sw_timer_destroy( HANDLE handle )
{
	if( NULL == (timer_nod_t *)handle )
	{
		return ;
	}
	//根据handle值，删除定时器链表中的node
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			delete_timer( ptimer );	
			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return ;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return ;
}


/**
 * @brief 重置定时器,状态切换到刚开始创建的状态
 * @param handle,定时器句柄
 *
 * @return 是否重置成功，0 成功，-1失败
 */
int sw_timer_reset( HANDLE handle )
{
	//根据handle，把指定node的times置为0；	
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{
		return -1 ;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == handle )
		{
			ptimer->times = ptimer->original_times;
			ptimer->start_time = sw_thrd_get_tick();  //设置start_time为系统当前时间
			ptimer->trigger_time = ptimer->start_time + ptimer->interval; 
			ptimer->status = TIMER_STATUS_ENABLE;

			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return 0;	
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return -1;
}

/** 
 * @brief 取得定时器状态
 * @param handle,定时器句柄
 *
 * @return 成功，返回定时器状态；失败返回-1
 */
timer_status_t sw_timer_get_status( HANDLE handle )
{

	//根据handle,获取node，返回指定node的定时器状态，也需要遍历定时器链表	
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1 ;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			sw_mutex_unlock( m_timer_des.htimer_mutex );	

			return ptimer->status;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex ); 

	return -1;
}

/** 
 * @brief 设置定时器状态
 * @param handle,定时器句柄
 *
 * @return 返回0表示设置成功，-1表示设置失败
 */
int sw_timer_set_status( HANDLE handle, timer_status_t status )
{

	//根据handle,获取node，返回指定node的定时器状态，也需要遍历定时器链表	
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;
	int now = 0;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1;
	}

	//判断status的输入值是否有错误
	if( !( status == 0 || status == 1 ) )
	{
		UTIL_LOG_ERROR( "param status is invalid value" );

		return -1;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			UTIL_LOG_INFO("find the handle1");
			if( ptimer->status == TIMER_STATUS_ENABLE && status == TIMER_STATUS_DISABLE  )
			{
				now = sw_thrd_get_tick( ); 
				ptimer->store_time = ptimer->trigger_time -now;

				ptimer->status = status;
		//		UTIL_LOG_INFO( "###%d\n,$$%lu,&&&%lu ",ptimer->status, ptimer->trigger_time ,ptimer->store_time );
				sw_mutex_unlock( m_timer_des.htimer_mutex );

				return 0;
			}

			else if( ptimer->status == TIMER_STATUS_DISABLE && status ==TIMER_STATUS_ENABLE )
			{
				now = sw_thrd_get_tick( );
				ptimer->trigger_time = ptimer->store_time + now;
				ptimer->status = status;

			//	UTIL_LOG_INFO( "###%d\n,$$$%lu,&&&%lu ",ptimer->status, ptimer->trigger_time ,ptimer->store_time );
				sw_mutex_unlock( m_timer_des.htimer_mutex );	

				sw_signal_give( m_timer_des.htimer_signal );

				return 0;

			}

			else if( ( ptimer->status == TIMER_STATUS_ENABLE && status ==TIMER_STATUS_ENABLE )||
						( ptimer->status == TIMER_STATUS_DISABLE && status ==TIMER_STATUS_DISABLE ) )
			{
				ptimer->status = status;

			//	UTIL_LOG_INFO( "##%d\n,$$%lu ",ptimer->status, ptimer->trigger_time );

				sw_mutex_unlock( m_timer_des.htimer_mutex );	

				return 0;
			}

	/*		else if( ptimer->status == TIMER_STATUS_DISABLE && status ==TIMER_STATUS_DISABLE )
			{
				ptimer->status = status;

				UTIL_LOG_INFO( "###%d\n,$$$%lu",ptimer->status, ptimer->trigger_time );

				sw_mutex_unlock( m_timer_des.htimer_mutex );	

				return 0;
			}
			*/
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex ); 

	return -1;
}

/**
 * @brief 根据名称取得定时器句柄
 * @param name, timer名称
 *
 *	@return 指定名称的定时器句柄,成功; NULL，失败
 */
HANDLE sw_timer_get_byname( char* name )
{

	timer_nod_t* ptimer = NULL;
	
	if( NULL == name )
	{
		return NULL;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( strcmp( ptimer->name , name ) == 0 )
		{
			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return (HANDLE)ptimer;	
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return NULL;
}

/**
 * @brief 定时处理函数
 * @param handle,定时器句柄
 * @param handler,定时器回调函数
 * @param wparam,定时器回调参数1
 * @param lparam,定时器回调参数2
 *  
 * @return 是否重置成功，0 成功，-1失败
 */
int sw_timer_set_ontimehandler( HANDLE handle,ontimehandler_t handler,uint32_t wparam,uint32_t lparam )   //重新SET 回调函数，使handler指向其他处理
{
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1;
	}
	
	//判断handler是否合法
	if( NULL == (ontimehandler_t)handler )
	{
		UTIL_LOG_ERROR( "param handler is invalid value(NULL)" );

		return -1;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );  

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer == timer_h )
		{
			ptimer->handler = handler;
			ptimer->wparam = wparam;
			ptimer->lparam = lparam;
			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return 0;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return -1;
}

/**
 * @brief 设置定时器定时时间,该函数会导致定时器重置
 * @param handle,定时器句柄
 * @param interval, 定时器时间间隔  
 * @param times,定时器重复次数，-1表示无限次
 *
 * @return 是否重置成功，0 成功，-1失败
 */
int sw_timer_set_interval( HANDLE handle,int interval,int times )
{
	timer_nod_t* ptimer = NULL;
	timer_nod_t* timer_h = NULL;
	timer_h = (timer_nod_t*)handle;

	if( NULL == (timer_nod_t *)handle )
	{   
		return -1;
	}
	//判断interval和times输入的合法性
	if( interval < 0 )
	{
		UTIL_LOG_ERROR( "param interval is invalid value" );

		return -1;
	}	
	if( times < -1 )
	{
		UTIL_LOG_ERROR( "param times is invalid value" );

		return -1;
	}

	sw_mutex_lock( m_timer_des.htimer_mutex );  

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( ptimer  == timer_h )
		{
			ptimer->times = times;
			ptimer->original_times = times;
			ptimer->start_time = sw_thrd_get_tick( );  //设置start_time为系统当前时间
			ptimer->interval=interval;
			ptimer->trigger_time = ptimer->start_time + ptimer->interval; 
			ptimer->status = TIMER_STATUS_ENABLE;

			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return 0;
		}
	}

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return -1;	
}

/**
  ＠brief 打印所有定时器信息
 * @return 无
 */
void sw_timer_print()
{
	timer_nod_t* ptimer = NULL;

	sw_mutex_lock( m_timer_des.htimer_mutex );  

	for ( ptimer = m_timer_des.ptimer_head; ptimer != NULL; ptimer = ptimer->next )
	{
		if( NULL == ptimer->next )
		{
			UTIL_LOG_INFO( "name is %s, interval=%lu, original_times=%ld,  start_time=%lu, status=%d \n ",
					ptimer->name, ptimer->interval, ptimer->original_times, ptimer->start_time, ptimer->status );

			sw_mutex_unlock( m_timer_des.htimer_mutex );

			return ;
		}
		else
		{
			UTIL_LOG_INFO( "name is %s, interval=%lu, original_times=%ld,  start_time=%lu, status=%d \n ",
					ptimer->name, ptimer->interval, ptimer->original_times, ptimer->start_time, ptimer->status );
		}

	}
	UTIL_LOG_INFO("the timer list is empty \n ");		

	sw_mutex_unlock( m_timer_des.htimer_mutex );

	return ;
}




