

#include <stdio.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "../src/listener.h"

int
main(int argc, char *argv[])
{
	int i;
	listener_pt p_listener;


	p_listener = listener_ctor("0.0.0.0", 8081);
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

	printf("Listening on fd %d, presss spacebar to terminate.\n", listener_get_fd(p_listener));
	getchar();

	listener_dtor(&p_listener);
	return 0;
}


