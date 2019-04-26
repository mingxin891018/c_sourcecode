#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define FILE_PATH "./dup.txt"
void main(void)
{
	int n = 0;
	int fd = open(FILE_PATH,O_RDWR|O_CREAT|O_TRUNC);
	printf("fd = %d-----test----\n",fd);
	dup2(fd, 0); 
	dup2(fd, 1); 
	dup2(fd, 2); 
	if (fd > 2)
		close(fd);
	while(1){
		printf("%d-----test----\n",n++);
		sleep(1);
	}

}




