#ifndef RELAYER_H__
#define RELAYER_H__

#define REL_JOBMAX		1024

enum
{
	STATE_RUNNING=1,
	STATE_CANCELED,
	STATE_OVER
};

struct rel_stat_st
{
	int fd1,fd2;
	int state;
	int64_t count12,count21;
//	struct timeval start,end;
};

int rel_addjob(int fd1,int fd2);
/*
 *	return	>= 0			成功，返回任务ID
 *			== -ENOMEM		失败，内存分配失败
 *			== -EINVAL		失败，参数非法
 *			== -ENOSPC		失败，任务数组满
 **/


int rel_canceljob(int id);
/*	
 *	return	== 0			成功，指定任务已取消
 *			== -EINVAL      失败，参数非法
 *			== -EBUSY		失败，指定任务已结束
 *			== -ECANCELED	失败，指定任务早已被取消		
 * */

int rel_statjob(int id,struct rel_stat_st *);
/*	
 *	return  == 0            成功，指定任务状态已返回
 *			== -EINVAL      失败，参数非法
 * */

int rel_waitjob(int id,struct rel_stat_st *);
/*  
 *  return  == 0            成功，指定任务状态已释放
 *          == -EINVAL      失败，参数非法
 * */


#endif

 

