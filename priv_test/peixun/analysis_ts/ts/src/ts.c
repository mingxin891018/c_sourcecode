#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"

//查找最近同步点
int ts_rewind(FILE *fp)
{
	ssize_t ret = 0;
	unsigned char head = '\0';
	if(fp == NULL)
	{
		printf("ts_rewind():argument error!\n");
		return -1;
	}
	rewind(fp);
	if(fread(&head,1,1,fp) != 1)
		return -1;
	while(head != 0x47)
	{
		if((ret = fread(&head,1,1,fp)) < 0)
			return -1;
		else if(ret == 0)
		{	
			printf("找不到同步点");
			break;
		}
	}
	fseek(fp,-1,SEEK_CUR);	
	return 0;
}

//读取ts文件
int read_ts_packet(FILE *fp,unsigned char *packet,int len) 
{
	int p = 0;
	if((fp == NULL)||(packet == NULL)||(len <= 0 ))
	{
		printf("read_ts_packet():argument error!\n");
		return -1;
	}
	p = fread(packet,1,PACKET,fp);
	if(p != PACKET)
	{
		perror("fgets()");
		return -1;
	}
	return 0;	
}

//解析ts文件
int parse_ts(const unsigned char *buffer,int file_size)
{
	unsigned short pat_pid = 0;
	if((buffer == NULL)||(file_size <= 0))
	{
		printf("read_ts_packet():argument error!\n");
		return -1;
	}
	if(buffer[0] != 0x47)
		return 0;
	
	pat_pid = (((buffer[1] & 0x1f) << 8) | buffer[2]);
	if(pat_pid != 0x0)
		return 0;
	else 
		return 1;
	return 0;
}

//解析pat表
int parse_pat(const unsigned char *buffer, ts_st *ts)
{
	int i = 0,j = 0,num = 0,pid = 0;
	int res = 0;
	short enable_bytes = 0;
	
	if((buffer == NULL)||(ts == NULL))
	{
		printf("parse_pat():argument error!\n");
		return -1;
	}
	
	enable_bytes = (buffer[6] << 8 | buffer[7])&0xfff;
	
	num = (enable_bytes - 9)/4;
	for(i = 0;i < num;i++)
	{
		//解析频道号码
		ts->programs[j].program_number = buffer[12+ i * 4 + 1] << 8 | buffer[13 + i * 4 + 1];
		//解析频道PID
		ts->programs[j].pmt_pid = ((buffer[14+ i * 4 + 1] << 8) | buffer[15 + i * 4 + 1])&0xfff;		
		pid = ts->programs[j].pmt_pid;
		res = ((pid != 0x10)&&(pid != 0x01)&&(pid != 0x02)&&(pid != 0x12)&&(pid!=0x13)&&(pid!=0x14));
		if(res)	
			j++;

	}
	ts->number_program = j;
	return 0;
}

//找出pmt表
int find_pmt(unsigned short pmt_pid,ts_st *ts)
{
	int i = 0;
	unsigned short pid = 0;
	if(ts == NULL)
	{
		printf("find_pmt():argument error!\n");
		return -1;
	}
	rewind(ts->fd);
	for(i = 0;i < ts->file_size/PACKET;i++)
	{
		memset(ts->pmt_buffer,0,PACKET);
		fread(ts->pmt_buffer,1,PACKET,ts->fd);
		if(ts->pmt_buffer[0] != 0x47)
			continue;

		pid = (((ts->pmt_buffer[1] & 0x1f) << 8) | ts->pmt_buffer[2]);
		if(pid != pmt_pid)
			continue;
		else 
			return 0;
	}
	return -1;
}

//解析pmt表
int parse_pmt(const unsigned char *buffer,int len,ts_st *ts,int temp)
{

	unsigned short enable_bytes = 0;
	int length = 0,i = 0,cur = 0;
	unsigned int program_info_length = 0;
	unsigned int ES_info_length = 0;
	unsigned int type = 0,elem_pid = 0;
	
	if((buffer == NULL)||(len <= 0)||(ts == NULL)||(temp <0))
	{
		printf("parse_pmt():argument error!\n");
		return -1;
	}
	//有效字节数
	enable_bytes = (buffer[6] << 8 | buffer[7])&0xfff;
	//描述字节数
	program_info_length = (buffer[15] << 8 | buffer[16]) & 0xfff;

	cur = 17 + program_info_length;
	length = enable_bytes - 13 - program_info_length;

	while(length > 0)
	{
		type = buffer[cur];
		elem_pid = (buffer[cur + 1]<<8|buffer[cur+2])&0xfff;
		//频道所含PMT的类型
		ts->programs[temp].program_list[i].stream_type = type;
		//频道所含流文件的PID号码
		ts->programs[temp].program_list[i].elementary_PID = elem_pid;
		//描述字符长度
		ES_info_length = (buffer[cur + 3] << 8 | buffer[cur + 4])&0xfff;
		cur += (5 + ES_info_length);
		length -= (5 + ES_info_length);
		i++;
	}
	ts->programs[temp].number_program_list = i;
	return 0;
}

//查找sdt表
int find_sdt(const unsigned char *buffer,ts_st *ts)
{
	unsigned int sdt_pid = 0;
	unsigned short table_id = 0;

	if((buffer == NULL)||(ts == NULL))
	{
		printf("find_sdt():argument error!\n");
		return -1;
	}
	sdt_pid = (((buffer[1] & 0x1f) << 8) | buffer[2]);
	table_id = buffer[5];

	if(buffer[0] != 0x47)
		return 0; 
	if((sdt_pid != 0x11)||(table_id != 0x42))
		return 0;
	else 
	{	
		memmove(ts->sdt_buffer,buffer,PACKET);
		return 1;
	}
	return 0;
}

//解析sdt表
int parse_sdt(const unsigned char *buffer,ts_st *ts)
{	
	int length = 0,cur = 16;
	unsigned short section_length = 0;
	unsigned short discriptor_loop_length = 0;
	unsigned short name_length1 = 0;
	unsigned short name_length2 = 0;
	int temp = 0;
	
	if((buffer == NULL)||(ts == NULL))
	{
		printf("find_sdt():argument error!\n");
		return -1;
	}
	section_length = (buffer[6] << 8 | buffer[7])&0xfff;
	length = section_length - 12;
	while(length > 0)
	{
		//频道的播放状态
		ts->programs[temp].running_status = buffer[cur + 3]>>4&0xe;
		//频道的加密状态
		ts->programs[temp].free_ca_mode = buffer[cur + 3]&0x1;
		//描述此频道的数据长度
		discriptor_loop_length = (buffer[cur + 3]<<8|buffer[cur + 4])&0xfff;
		name_length1 = buffer[cur + 8];
		memmove(ts->programs[temp].discriptor_buffer[0],buffer + cur + 9,name_length1);
		name_length2 = buffer[cur + name_length1 + 9];
		memmove(ts->programs[temp].discriptor_buffer[1],buffer + cur + name_length1 +10 ,name_length2);
		cur += (5 + discriptor_loop_length);
		length -= (5 + discriptor_loop_length);
		temp++;
	}
	return 0;
}

void print_pid_type(unsigned int type)
{
	if(type == 0x2)
		printf("视频文件PID:");
	else if(type == 0x4)
		printf("音频文件PID:");
	else if(type == 0x6)
		printf("private文件PID:");
	else 
		printf("未知PID:");
}

int ts_print(ts_st *ts)
{
	int i = 0,j = 0,k = 0;
	if(ts == NULL)
	{
		printf("ts_print():argument error!\n");
		return -1;
	}
	printf("此TS流文件共有%d个频道:\n\n",ts->number_program);
	for(i = 0;i < ts->number_program;i++)
	{
		printf("频道%d:\n",ts->programs[i].program_number);
		printf("频道名1:%s\n原码:",ts->programs[i].discriptor_buffer[0]);
		for(j = 0;ts->programs[i].discriptor_buffer[0][j] != '\0';j++)
			printf("%x ",ts->programs[i].discriptor_buffer[0][j]);
		printf("\n");
		printf("频道名2:%s\n原码:",ts->programs[i].discriptor_buffer[1]);
		for(j = 0;ts->programs[i].discriptor_buffer[1][j] != '\0';j++)
			printf("%x ",ts->programs[i].discriptor_buffer[1][j]);
		printf("\n");
		for(k = 0;k < ts->programs[i].number_program_list;k++)
		{	
			print_pid_type(ts->programs[i].program_list[k].stream_type);
			printf("0x%x\n",ts->programs[i].program_list[k].elementary_PID);
		}
		printf("加密状态:%s  ",(ts->programs[i].free_ca_mode?"加密":"未加密"));
		printf("播放状态:");
		switch(ts->programs[i].running_status)
		{
			case 1:printf("还未播放\n");break;
			case 2:printf("数分钟内播放\n");break;
			case 3:printf("播放暂停\n");break;
			case 4:printf("正在播放\n");break;
			default:printf("未知\n");break;
		}
		printf("\n");
	}
	return 0;
}


