#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void alrm_handler(int s)
{
	alarm(1);
	puts("Hello World.");

	return ;
}

int main()
{
	signal(SIGALRM,alrm_handler);
	alarm(1);


	while(1)
		pause();

/*	
	signal(SIGALRM,alrm_handler);
	alarm(3);


	pause();
	puts("Hello World");
*/
	

/*
//	signal(SIGALRM,alrm_handler);
	alarm(5);
	alarm(1);
	alarm(10);

	while(1);
*/
	exit(0);
}


