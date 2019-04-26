#include <stdio.h>
#include <string.h>

int main(int argc,char **argv)
{
	char buf[64];
	//unsigned int a = 2247483648; 
	unsigned int a = 4294967295; 
	printf("a_d = %d\n",a);
	printf("a_u = %u\n",a);
	
	memset(buf,0,64);
	sprintf(buf,"%d",a);
	printf("a=%s\n",buf);
	
	memset(buf,0,64);
	sprintf(buf,"%u",a);
	printf("a=%s\n",buf);
}





