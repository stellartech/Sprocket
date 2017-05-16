

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "../src/listener.h"

int
main(int argc, char *argv[])
{
	int i, n, epoll_fd;
	listener_pt p_listener;
	struct epoll_event *a_events;

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

	epoll_fd = epoll_create1(0);
	if(epoll_fd == -1) {
		printf("Epoll created failed\n");
		listener_dtor(&p_listener);
		return -1;
	}
	else {
		struct epoll_event event;
		int sfd = listener_get_fd(p_listener);
		event.data.fd = sfd;
		event.events = EPOLLIN | EPOLLET;
		if(epoll_ctl (epoll_fd, EPOLL_CTL_ADD, sfd, &event) == -1) {
			printf("Epoll add listener fd failed\n");
			close(epoll_fd);
			listener_dtor(&p_listener);
			return -1;
		}
	}

	a_events = calloc(64, sizeof(struct epoll_event));

	while(1) {
		struct epoll_event *events;
		n = epoll_wait(epoll_fd, a_events, 64, -1);
		for(i = 0; i < n; i++) {
			if((a_events[i].events & EPOLLERR) ||
				(a_events[i].events & EPOLLHUP) ||
				(!(a_events[i].events & EPOLLIN))) {
				fprintf(stderr, "Epoll errored\n");
				close(a_events[i].data.fd);
				continue;
			}
			else if(a_events[i].data.fd == listener_get_fd(p_listener)) {
				

			}
		}

	}
	
	
	if(a_events) free(a_events);
	


	listener_listen(p_listener);

	printf("Listening on fd %d, presss spacebar to terminate.\n", listener_get_fd(p_listener));
	getchar();

	listener_dtor(&p_listener);
	return 0;
}


