#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct tmp {
	char buf[32];
	int name;
	char value[32];
} tmp_st;

void main(void)
{
	tmp_st *temp = NULL;
//	temp = malloc(sizeof(struct tmp));
	printf("%s",temp->value);

}
