#include <stdio.h>
#include <string.h>


int main(void)
{
	char * buf="sdfmingxinsdfdsf";
	char *p =NULL;
	
	p = strstr(buf,"mngxin");
	if(p == 0)
		printf("p=%d++++++++++++++++++++++\n",p);
	else{
		printf("ret=%p-----------\n",p);
	}
	return 0;
}

