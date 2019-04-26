#include <stdio.h>


typedef struct {
	int a;
	int c;
	char b[];
	char d[];
}m;
main()
{
	m a;
	printf("sizeof(m)=%d\n",sizeof(m));
}
