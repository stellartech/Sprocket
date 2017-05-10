

#include <stdio.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

typedef struct
{
	int fd;
	struct sockaddr in_addr;
	void *p_userdata;	
} connection_t, *connection_pt;


int
main(int argc, char *argv[])
{
	printf("Hello world\n");
	return 0;
}


