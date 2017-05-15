

#include <stdio.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "../src/listener.h"

int
main(int argc, char *argv[])
{
	int i;
	listener_pt p_listener;


	//p_listener = listener_ctor("0.0.0.0", 8081);
	p_listener = listener_ctor("172.17.0.2", 8081);
	//p_listener = listener_ctor("127.0.0.1", 8081);
	if(!p_listener) {
		printf("Failed ctor()\n");
		return 0;
	}
	i = listener_bind(p_listener);
	if(i < 1) {
		printf("Failed bind %d\n", i);
		listener_dtor(&p_listener);
		return 0;
	}
	listener_set_backlog(p_listener, 1024);
	listener_listen(p_listener);

	printf("Hello world\n");
	getchar();

	listener_dtor(&p_listener);
	return 0;
}


