/*************************************************************************
	> File Name: main.c
	> Author: LFJ
	> Mail: 
	> Created Time: 2018年09月05日 星期三 13时48分17秒
 ************************************************************************/

#include <stdio.h>
#include "pthread.h"
#include "mqtt.h"
#include "unistd.h"
#include "string.h"

int main(int argc, char *argv[])
{
	char send_buf[512] = {0};
    pthread_t thread_ID;		//定义线程id

    pthread_create(&thread_ID, NULL, &cloud_mqtt_thread, NULL);	//创建一个线程执行mqtt客户端
    pthread_detach(thread_ID);	//设置线程结束收尸

	memset(send_buf, 0, sizeof(send_buf));
	strcpy(send_buf, "this is my message!");
    
	while (1) {
        sleep(5);						//睡眠3s
        mqtt_data_write(send_buf, strlen(send_buf), 0);//循环发布"my yes"
    }

    return 0;
}
