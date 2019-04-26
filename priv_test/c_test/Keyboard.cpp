///
/// \copyright Copyright © 2017 ActiveVideo, Inc. and/or its affiliates.
/// Reproduction in whole or in part without written permission is prohibited.
/// All rights reserved. U.S. Patents listed at http://www.activevideo.com/patents
///

#include <porting_layer/Keyboard.h>

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>

using namespace ctvc;

namespace {
class TerminalModeSetter
{
public:
    TerminalModeSetter()
    {
        // Save the current terminal settings.
        tcgetattr(0, &m_orig_termios);

        // Create a new terminal mode from the old and set it into raw mode.
        struct termios new_termios;
        memcpy(&new_termios, &m_orig_termios, sizeof(new_termios));
        cfmakeraw(&new_termios);

        // Keep the original output modes.
        new_termios.c_oflag = m_orig_termios.c_oflag;

        // Make sure our inputs still pass signal keys.
        new_termios.c_lflag |= ISIG;

        // And set the new terminal mode.
        tcsetattr(0, TCSANOW, &new_termios);
    }

    ~TerminalModeSetter()
    {
        // Restore the original terminal settings.
        tcsetattr(0, TCSANOW, &m_orig_termios);
    }

private:
    struct termios m_orig_termios;
};

static TerminalModeSetter s_terminal_mode_setter;
} // namespace


#define CLOUD_PATH "/tmp/cloud.tmp"

static int server_sockfd = -1;
static int accept_sockfd = -1;
struct sockaddr_un server_address;

int Keyboard::get_key()
{
	int ret = 0;
	int tmp = 0;
	int key = 0;
	char buf[128] = {0};
	char *p = NULL;
	
	memset(buf,0,sizeof(buf));
	if(server_sockfd == -1){
		unlink(CLOUD_PATH);
		server_sockfd = create_cloud_sock(CLOUD_PATH);
		if(server_sockfd < 0)
			return 0;
	}
	accept_sockfd = wait_accept_sock(server_sockfd);
	if(accept_sockfd < 0)
		return 0;
	ret = read_data(accept_sockfd, buf,sizeof(buf));
	if(ret < 0)
		return 0;
	close(accept_sockfd);
	accept_sockfd = -1;

	p = strstr(buf,"=");
	tmp = atoi(p+1);
	printf("cloud tmp=%d\n",tmp);
	switch(tmp){
		case 273:printf("cloud_key: UP\n");key = UP_KEY; break;
		case 274:printf("cloud_key: DOWN\n");key = DOWN_KEY; break;
		case 275:printf("cloud_key: RIGTH\n");key = RIGHT_KEY; break;
		case 276:printf("cloud_key: LEFT\n");key = LEFT_KEY; break;
		case  13:printf("cloud_key: ENTER\n");key = ENTER_KEY; break;
		case 331:printf("cloud_key: BACK\n");key = BACKSPACE_KEY; break;
		default:
			key = 0;
		
	}

	return key;
}


int Keyboard::create_cloud_sock(const char *path_name)
{
	int server_len = 0;
	unlink (path_name); /*删除原有server_socket对象*/
	server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0); 
	if(server_sockfd < 0)
	{   
		perror("socket");
		return -1; 
	}   
	server_address.sun_family = AF_UNIX;
	strcpy (server_address.sun_path, path_name);
	server_len = sizeof (struct sockaddr_un);

	if(bind (server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
	{   
		perror("bend");
		return -1; 
	}   
	listen (server_sockfd, 5); 
	return server_sockfd;
}


int Keyboard::wait_accept_sock(int server_sockfd)
{
	int client_len = sizeof(struct sockaddr_un);
	accept_sockfd = accept (server_sockfd, (struct sockaddr *)&server_address, (socklen_t *)&client_len);
	if(accept_sockfd < 0)
	{   
		perror("accept");
		return -1; 
	}   
	return accept_sockfd;
}

int Keyboard::read_data(int accept_sockfd,char *buf,int len)
{
	int bytes = 0;
	if ((bytes = read (accept_sockfd, buf, len)) == -1) {
		perror("read");
		return -1;
	}
	return bytes;
}


