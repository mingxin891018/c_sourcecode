#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h> 
#include <stdint.h>
#include "ts.h"

int main(int argc,char *argv[])
{
	int flags_pat = 0;
	int flags_sdt = 0;
	char *p = NULL;
	int i = 0,ret = 0;
	char pathname[NAMESIZE];
	static ts_st *ts;

	ts = malloc(sizeof(*ts));
	if(ts == NULL)
	{
		perror("malloc()");
		return -1;
	}
	memset(ts,0,sizeof(*ts));

	p = getcwd(pathname,NAMESIZE);
	if(p == NULL)
	{
		perror("getcwd()");
		return -1;
	}

	strncat(pathname,"/",1);
	strncat(pathname,FILENAME,strlen(FILENAME));
	printf("\nTS流文件是：%s\n",strrchr(pathname,'/')+1);
	ts->fd = fopen(pathname,"r");
	if(ts->fd == NULL)
	{
		perror("open()");
		return -1;
	}
	fseek(ts->fd,0,SEEK_END);
	ts->file_size = ftell(ts->fd);
	printf("长度为：%ld\n",ts->file_size);
	
	ts_rewind(ts->fd);
	ts->file_size -= ftell(ts->fd);
	for(i = 0;i < ts->file_size/PACKET; i++)
	{
		if(read_ts_packet(ts->fd,ts->buffer,PACKET) < 0)
		{
			printf("read_ts_packet():读取文件错误\n");
			return -1;
		}	
		ret = parse_ts(ts->buffer,PACKET);
		if(ret)
		{
			parse_pat(ts->buffer,ts);
			flags_pat = 1;
		}
		ret = find_sdt(ts->buffer,ts);
		if(ret)		
			flags_sdt = 1;
	}
	if(!flags_pat)
	{
		printf("找不到pat包\n");
		return -1;
	}

	if(flags_sdt)
		parse_sdt(ts->sdt_buffer,ts);
	else
		printf("找不到pat包\n");
	for(i = 0;i < ts->number_program; i++)
	{
		ret = find_pmt(ts->programs[i].pmt_pid,ts);
		if(ret == -1)
		{
			printf("找不到第%d个pmt数据包\n",i);
			continue;
		}
		parse_pmt(ts->pmt_buffer,PACKET,ts,i);			
	}	
	fclose(ts->fd);
	ts_print(ts);
	free(ts);
	return 0;
}


