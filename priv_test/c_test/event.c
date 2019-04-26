#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <event.h>

//gcc event.c -o event -levent
static void cmd_event(int fd, short event, void *arg)
{
	char msg[1024] = {0};
	int ret = read(fd, msg, sizeof(msg));
	if(ret <+ 0){
		perror("read fail\n");
		return ;
	}
	msg[ret] = '\0';
	printf("mgs=%s\n", msg);
}

int main(int argc, char **argv)
{
	struct event ev_cmd;
	event_init();
	
	event_set(&ev_cmd, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_event, NULL);
	event_add(&ev_cmd, NULL);
	event_dispatch();
	
	return 0;
}



