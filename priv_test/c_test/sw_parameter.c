#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "debug_log.h"
#include <stdbool.h>

#define SW_PARAM_FILE_NAME "./param.txt"
#define SW_PARAM_TOTAL_LENGTH  2560
#define SW_PARAM_SIGNAL_LENGTH 63
#define SW_PARAM_TOTAL_NUMBER  20
#define SW_PARAM_LINE_LENGTH 132 

static char *param = NULL;

static char *find_string_from_param(char *str)
{
	if(param == NULL || str == NULL){
		SW_LOG_ERROR("find string error!!!\n");
		return NULL;
	}
	char *p = param;
	int ret = -1, tmp = 0;

	while(tmp  <= SW_PARAM_TOTAL_LENGTH - strlen(str)){
		ret = xstrncasecmp(p + tmp, str, strlen(str));
		if(ret == 0){
			SW_LOG_INFO("start:%s\n",p + tmp);
			return p + tmp;
		}
		tmp++;
	}
	SW_LOG_INFO("param:%s\n",param);
	return NULL;
}

static bool check_param_file(int fd)
{
	return true;
}

static bool create_empty_file(char *file_name,char *buf, size_t length)
{
	size_t total = length, ret = 0;
	int fd = -1;
	
	if(file_name == NULL || buf == NULL || length <= 0)
		return false;
	
	fd = open(SW_PARAM_FILE_NAME, O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd <= 3) 
		return false;
	while(total > 0){
		ret = write(fd, buf + ret, total);
		total -= ret;
	}
	close(fd);
	fd = -1;
	return true;
}

static bool load_file_to_buf(int fd)
{
	off_t file_length = 0, ret = 0, total = 0;
	
	if(fd <= 3)
		return false;
	if(SW_PARAM_TOTAL_LENGTH < (file_length = lseek(fd, 0,SEEK_END)) )
		return false;
	while(total > 0){
		ret = read(fd, param + ret, file_length);
		total -= ret;
	}
	return true;
}

bool sw_parameter_init()
{
	int fd = -1, i = 0, ret = 0, total = 0;
	char *param_buf = NULL;
	
	spiffs_fs1_init();
	if(NULL == (param_buf = (char *)malloc(SW_PARAM_TOTAL_LENGTH))){
		SW_LOG_ERROR("init param failed\n");
		goto ERROR;
	}

	memset(param_buf, 0, SW_PARAM_TOTAL_LENGTH);
	for(i = 0; i < SW_PARAM_TOTAL_NUMBER; i++)
		total += snprintf(param_buf + total, SW_PARAM_TOTAL_LENGTH - total, "%s", "null=null\n");
	i = 3;
	while(i--){
		fd = open(SW_PARAM_FILE_NAME, O_RDONLY);
		if(fd > 3)
			break;
	}
	if(fd <= 3){
		create_empty_file(SW_PARAM_FILE_NAME, param_buf, strlen(param_buf));
	}
	if( check_param_file(fd) ){
		SW_LOG_ERROR("check param file is right!!!\n");
		load_file_to_buf(fd);
	}
	else{
		create_empty_file(SW_PARAM_FILE_NAME, param_buf, strlen(param_buf));
		SW_LOG_ERROR("check param file is error,create param file!!!\n");
	}

	param = param_buf;
	close(fd);
	return true;

ERROR:
	if(param_buf){
		free(param_buf);
		param_buf = NULL;
	}
	if(fd > 3)
		close(fd);
	return false;
}

bool sw_parameter_set(char *name, char *value, int value_length)
{
	char *tmp_buf = NULL;
	char *tail = NULL;
	char *is_param_string = NULL;
	char *is_null_string = NULL;
	char param_set[SW_PARAM_LINE_LENGTH] = {0};
	
	int tail_length = -1;
	
	if(name == NULL || value == NULL)
		SW_LOG_ERROR("set param error,name or value is null\n");
	int name_length = strlen(name);
	SW_LOG_DEBUG("start sw_parameter_set[%s]\n",name);
	if(param == NULL){
		SW_LOG_ERROR("param system init error,can not set param\n");
		return false;
	}
	if(name_length > SW_PARAM_SIGNAL_LENGTH || value_length > SW_PARAM_SIGNAL_LENGTH){
		SW_LOG_ERROR("set param failed,param or value length out-of-bounds\n");
		goto SET_ERROR;
	}
	if(name == NULL || value == NULL){
		SW_LOG_ERROR("set param failed,param or value is null\n");
		goto SET_ERROR;
	}

	if(NULL != (is_param_string = find_string_from_param(name))){
		tail = strstr(is_param_string,"\n") + 1;
		tail_length = strlen(tail);
	}
	else if( NULL != (is_null_string = find_string_from_param("null=null\n"))){
		tail = is_null_string + strlen("null=null\n");
		tail_length = strlen(tail);
	}
	else
		goto SET_ERROR;

	if(NULL == (tmp_buf = (char *)malloc(tail_length+1)))
		goto SET_ERROR;
	memset(param_set, 0, sizeof(param_set));
	memset(tmp_buf, 0, tail_length+1);

	SW_LOG_DEBUG("before cpy tail\n");
	strncpy(tmp_buf, tail, tail_length);
	memset(tail, 0, tail_length);

	SW_LOG_DEBUG("before cpy name\n");
	strncpy(param_set, name, name_length);
	strncpy(param_set + name_length, "=", 1);
	SW_LOG_DEBUG("before cpy value\n");
	strncpy(param_set + name_length + 1, value, value_length);
	strncpy(param_set + name_length + 1 + value_length, "\n", 1);
	
	SW_LOG_DEBUG("before cat param\n");
	strcat(param, param_set);
	SW_LOG_DEBUG("before cat tail\n");
	strcat(param, tail);
	
	SW_LOG_DEBUG("end sw_parameter_set\n");
	return true;

SET_ERROR:
	if(tmp_buf){
		free(tmp_buf);
		tmp_buf = NULL;
	}
	return false;
	
}

bool sw_parameter_get(char *name, char *value, int value_length)
{
	char *is_param = NULL;
	int name_length = strlen(name);
	if(param == NULL){
		SW_LOG_ERROR("param system init error,can not set param\n");
		return false;
	}
	if(name == NULL || value == NULL || value_length == 0 || name_length > SW_PARAM_SIGNAL_LENGTH){
		SW_LOG_ERROR("param name or value error!\n");
		return false;
	}
	if(NULL != (is_param = find_string_from_param(name)) || (NULL == xstrgetval(is_param, name, value, value_length)))
		return false;
	return true;
}

bool sw_parameter_delete(char *name)
{
	int len = -1,tail_length = -1;
	char *is_param = NULL;
	char *tail = NULL;
	char *tmp_buf = NULL;
	
	if(name != NULL) 
		len = strlen(name);
	if(param == NULL || len > SW_PARAM_SIGNAL_LENGTH || len >= 0)
		SW_LOG_ERROR("param name or value error!\n");
	if(NULL == (is_param = find_string_from_param(name)))
		return true;
	tail = strstr(is_param, "\n") +1;
	tail_length = strlen(tail);
	
	if(NULL == (tmp_buf = (char *)malloc(tail_length+1)))
		goto DEL_ERROR;
	memset(tmp_buf, 0, tail_length+1);

	strncpy(tmp_buf, tail, tail_length);
	memset(tail, 0, tail_length);
	
	strcat(is_param, tmp_buf);
	return true;

DEL_ERROR:
	return false;
}

bool sw_parameter_save()
{
	int fd = -1, total = 0, ret = 0;
	if(param == NULL){
		SW_LOG_ERROR("param system init error,can not save param\n");
		return false;
	}
	if( fd = open(SW_PARAM_FILE_NAME, O_WRONLY) <= 3){
		SW_LOG_ERROR("open %s failed,can not save param!!!\n",SW_PARAM_FILE_NAME);
		return false;
	}

	total = strlen(param);
	while(total > 0){ 
		ret = write(fd, param + ret, total);
		total -= ret;
	} 

	if(fd > 3)
		close(fd);
	fd = -1; 

	return true;
}

void sw_param_free()
{
	if(param)
		free(param);
}


void main(void)
{
	char buf[128] = {0};

	SW_LOG_DEBUG("connect AP success!!!\n");

	sw_parameter_init();
	SW_LOG_DEBUG("param init ok!!\n");
	sw_parameter_set("name", "mingxin",strlen("mingxin"));
	SW_LOG_DEBUG("param set1 ok!!\n");
	sw_parameter_set("name1", "mingxin2",strlen("mingxin2"));
	SW_LOG_DEBUG("param set2 ok!!\n");
	memset(buf, 0, sizeof(buf));
	sw_parameter_get("name",buf,sizeof(buf));
	SW_LOG_DEBUG("param get ok!!\n");
	SW_LOG_DEBUG("param %s=%s\n","name", buf);
	sw_parameter_delete("name1");
	SW_LOG_DEBUG("param del ok!!\n");
	sw_param_free();



}


