#include <stdio.h>
#include <arpa/inet.h>


int main()
{
	int num = 0x11223344;
	printf("%x\n", htonl(num));
	printf("%x\n", ntohl(num));
	printf("%x\n", num);
	printf("kong=%s\n");
}
