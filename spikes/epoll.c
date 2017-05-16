
// A quick and dirty spike/poc to demo epoll 

#include <stdio.h>
#include <sys/epoll.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>

#include "../src/listener.h"

#define MAXEVENTS 64

static int
epoll_notification_accept(int epoll_fd, int in_fd, struct epoll_event *inp_event);
static int
epoll_data_event(int epoll_fd, struct epoll_event *inp_event);


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
		if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &event) == -1) {
			printf("Epoll add listener fd failed\n");
			close(epoll_fd);
			listener_dtor(&p_listener);
			return -1;
		}
		else {
			printf("Added epoll listener fd %d  to epoll fd %d\n", sfd, epoll_fd);
		}
	}

	a_events = calloc(MAXEVENTS, sizeof(struct epoll_event));

	listener_listen(p_listener);
	printf("Listening on fd %d, presss spacebar to terminate.\n", listener_get_fd(p_listener));

	while(1) {
		struct epoll_event *events;
		printf("Waiting for epoll event...\n");
		n = epoll_wait(epoll_fd, a_events, MAXEVENTS, -1);
		for(i = 0; i < n; i++) {
			if((a_events[i].events & EPOLLERR) ||
				(a_events[i].events & EPOLLHUP) ||
				(!(a_events[i].events & EPOLLIN))) {
				fprintf(stderr, "Epoll errored\n");
				close(a_events[i].data.fd);
				continue;
			}
			else if(a_events[i].data.fd == listener_get_fd(p_listener)) {
				printf("Epoll accept\n");
				epoll_notification_accept(epoll_fd, a_events[i].data.fd, &a_events[i]);	
			}
			else {
				printf("Data...\n");
				epoll_data_event(epoll_fd, &a_events[i]);
			}
		}

	}
	
	
	if(a_events) free(a_events);

	printf("getchar()\n");
	getchar();

	listener_dtor(&p_listener);
	return 0;
}

static int
epoll_data_event(int epoll_fd, struct epoll_event *inp_event)
{
	int rc;
	ssize_t count;
	char buf[512];

	// Drain the read buffer for the fd
	while(1) {
		memset(buf, 0, 512);
		count = read(inp_event->data.fd, buf, 512);
		if(count == -1 && errno == EAGAIN) {
			// We get this when the input buffer is empty.
			printf("    eagain on %d, ok\n", inp_event->data.fd);
			return 0;
		}		
		if(count == -1 && errno != EAGAIN) {
			printf("    fail not eagain on %d with %d, closing\n", inp_event->data.fd, errno);
			close(inp_event->data.fd);
			return 0;
		}
		if(count == 0) {
			printf("    remote closed connection on %d\n", inp_event->data.fd);
			close(inp_event->data.fd);
			return 0;
		}
		// Echo to sender
		write(inp_event->data.fd, buf, count);
		// Write to stdout too
		write(1, buf, count);
	}
	return 0;
}

static int 
epoll_notification_accept(int epoll_fd, int in_fd, struct epoll_event *inp_event)
{
	int rc, new_fd;
	struct sockaddr in_addr;
	socklen_t in_len = sizeof(struct sockaddr);
	struct epoll_event event;
	char hbuf[NI_MAXHOST];
	char sbuf[NI_MAXSERV]; 

	while(1) {
		if((new_fd = accept(in_fd, &in_addr, &in_len)) == -1) {
			return 0;
		}
		rc = getnameinfo(&in_addr, in_len, hbuf, NI_MAXHOST, sbuf, NI_MAXSERV,
			NI_NUMERICHOST | NI_NUMERICSERV);
		if(rc == 0) {
			printf("Accept %d - %s, %s\n", new_fd, hbuf, sbuf);
		}
		rc = fcntl(new_fd, F_GETFL, 0);
		rc |= O_NONBLOCK;
		fcntl(new_fd, F_SETFL, rc);
		event.data.fd = new_fd;
		event.events = EPOLLIN | EPOLLET;
		rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &event);	
	}
	return 0;
}
