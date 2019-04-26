#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void main(void)
{
	char *p = "";
	if (p == NULL)
		printf("yes\n");
	else
		printf("no\n");

	printf("p = %p\n",p);
	printf("p = %s\n",p);
}



