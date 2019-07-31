#include "swapi.h"
#include "swurl.h"
#include "swsignal.h"
#include "swmutex.h"
#include "swthrd.h"
#include "swgethostbyname.h"
struct _sw_info_t {
	void *signal;
	int is_exit;
	uint32_t ipaddr;
	char hostname[1024];
};
static void *m_mute = NULL;

#define LOCK() 		do {if (m_mute == NULL)m_mute = sw_mutex_create(); if (m_mute) sw_mutex_lock(m_mute);}while(0);
#define UNLOCK() 		do {if (m_mute) sw_mutex_unlock(m_mute);}while(0);
#define DEBUG()		printf("sw_gethostbyname2:%d:%p.\n",__LINE__,info);

static char *GetProcName();
static void FreeInfo(struct _sw_info_t *info);
static int Proc(struct _sw_info_t *info,uint32_t wpara);
uint32_t sw_gethostbyname2(char *hostname,int timeout) 
{
	uint32_t ipaddr = 0;
	void *thrd;
	struct _sw_info_t *info;
	if (hostname == NULL)
		return 0;
	info = malloc(sizeof(struct _sw_info_t));
	if (info == NULL)
		return 0;
	DEBUG();
	memset(info,0,sizeof(struct _sw_info_t));
	info->signal = sw_signal_create(0,1);
	info->is_exit = 0;
	info->ipaddr = 0;
	snprintf(info->hostname,sizeof(info->hostname),"%s",hostname);

	thrd = sw_thrd_open(GetProcName(),0,0,0,(void*)Proc,(uint32_t)info,(uint32_t)0);
	if (thrd == NULL) {
		DEBUG();
		free(info);
		return 0;
	}
	sw_thrd_resume(thrd);
	DEBUG();
	sw_signal_wait(info->signal,timeout);
	LOCK();
	ipaddr = info->ipaddr;
	DEBUG();
	FreeInfo(info);
	UNLOCK();
	return ipaddr;
}
static void FreeInfo(struct _sw_info_t *info) 
{
	if (info->is_exit == 0)
	{
		info->is_exit = 1;
		DEBUG();
		return ;
	}
	sw_signal_destroy(info->signal);
	DEBUG();
	free(info);
}
static char *GetProcName()
{
	static int count = 0;
	static char name[64] = {0};
	snprintf(name,sizeof(name),"gethostbyname%d",count++);
	return name;
}
static int Proc(struct _sw_info_t *info,uint32_t wpara)
{
	int rc = 0;
	char buf[1024*2] = {0};
	struct hostent host,*phost = NULL;
	DEBUG();
	if (gethostbyname_r(info->hostname,&host,buf,sizeof(buf),&phost,&rc) >= 0)
	{
		if ( phost != NULL && phost->h_addr_list != NULL && phost->h_addr_list[0] != NULL )   //增加判断，并使用 
		{
			DEBUG();
			memcpy(&info->ipaddr, phost->h_addr_list[0], sizeof(info->ipaddr));//ipaddr定义为unsigned long
		}
	}
	LOCK();
	sw_signal_give(info->signal);
	DEBUG();
	FreeInfo(info);
	UNLOCK();
	return 0;
}
