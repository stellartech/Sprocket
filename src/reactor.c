/*********************************************************************************
 *   Copyright (c) 2008-2017 Andy Kirkham  All rights reserved.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included
 *   in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 ***********************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/eventfd.h>

#include <time.h>
#include <stdio.h>

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

#include "reactor.h"

#ifdef __cplusplus
extern C {
#endif

struct _reactor
{
	// Epoll for the listener.
	// event.data.fd is the listening fd
	int		epoll_fd;

	// Epoll for connections.
	// event.data.ptr is the user's void*
	int		epoll_event_fd;
	uint32_t	event_flags;

	reactor_cb	p_cb;
	void		*p_userdata;

	int		num_of_events;
	struct epoll_event *p_events;
};

reactor_pt
reactor_ctor(reactor_ctor_args_pt inp_args)
{
	reactor_pt p_self = calloc(1, sizeof(reactor_t));
	if(p_self) {
		if((p_self->epoll_fd = epoll_create1(0)) == -1) {
			goto reactor_ctor_fail;	
		}
		if((p_self->epoll_event_fd = epoll_create1(0)) == -1) {
			goto reactor_ctor_fail;
		}
		p_self->num_of_events = 100;
		p_self->p_events = calloc(p_self->num_of_events, sizeof(struct epoll_event));
		if(!p_self->p_events) {
			goto reactor_ctor_fail;
		}
		if(inp_args) {
			reactor_react_to(p_self, inp_args->listener_fd);
			reactor_set_event_flags(p_self, inp_args->event_flags);
			reactor_set_userdata(p_self, inp_args->p_userdata);
			reactor_set_cb(p_self, inp_args->p_callback);
		}
		else {
			p_self->p_cb = NULL;
			p_self->p_userdata = NULL;
		}
	}
	return p_self;
	reactor_ctor_fail:
	reactor_dtor(&p_self);
	return p_self;
}

void
reactor_free(void *inp)
{
	if(inp) {
		reactor_pt p_self = (reactor_pt)inp;
		if(p_self->epoll_fd > 0) close(p_self->epoll_fd);
		if(p_self->epoll_event_fd > 0) close(p_self->epoll_event_fd);
		if(p_self->p_events) free(p_self->p_events);
		free(inp);
	}
}

void
reactor_dtor(reactor_pt *inpp)
{
	if(inpp) {
		reactor_pt p_self = *inpp;
		reactor_free(p_self);
		*inpp = NULL;
	}
}

static int
reactor_epoll_accept(reactor_pt inp_self, int listening_fd)
{
        int new_fd;
        socklen_t in_len;
        struct sockaddr in_addr;

        while((new_fd = accept(listening_fd, &in_addr, &in_len)) != -1) {
		if(inp_self->p_cb == NULL) {
			close(new_fd);
		}
		else {
                	int rc = fcntl(new_fd, F_GETFL, 0);
			if(rc < 0) {
				close(new_fd);
			}
			else {
				reactor_cb_args_t args;
				struct epoll_event event;
		                rc |= O_NONBLOCK;
        		        fcntl(new_fd, F_SETFL, rc);
				memset(&args, 0, sizeof(reactor_cb_args_t));
				args.type = REACTOR_ACCEPT;
				args.data.accept_args.errornumber = errno;
				args.data.accept_args.listener_fd = listening_fd;
				args.data.accept_args.accecpt_fd = new_fd;
				args.data.accept_args.in_len = in_len;
				args.data.accept_args.p_addr = &in_addr;
				args.data.accept_args.p_userdata = inp_self->p_userdata;
				(inp_self->p_cb)(&args); // Callback now.
				// Note, the callee should alter args.data.accept_args.p_userdata
				// to point at any new data struct it wants to use for thsi conn.
				// as it's what gets passed back later on events.
                		event.data.ptr = args.data.accept_args.p_userdata;
	                	event.events = inp_self->event_flags; 
	        	        rc = epoll_ctl(inp_self->epoll_event_fd, EPOLL_CTL_ADD, 
					new_fd, &event);
			}
		}
        }
        return 0;
}

int
reactor_loop_once_for(reactor_pt inp_self, int in_timeout)
{
	int t;
	struct epoll_event *p_event = NULL;

	if(!inp_self) {
		return -1;
	}

	if(in_timeout == 0) {
		t = 1;
		in_timeout = 1;
	}
	else {
		// Ensure timeout is even and then split the
		// the timeout across the two event_wait() calls.
		if((in_timeout % 2) == 1) {
			in_timeout++;
		}
		t = in_timeout / 2;
		in_timeout = 1;
	}

	for( ; t ; t--) {
		int n = 0;
		struct epoll_event *p_event = NULL;
		// Look for new connections.
		n = epoll_wait(inp_self->epoll_fd,
				inp_self->p_events, 
				inp_self->num_of_events, in_timeout);
		for(int i = 0; i < n; i++) {
			p_event = &inp_self->p_events[i];
			if(!(p_event->events & EPOLLIN)) {
				if(inp_self->p_cb) {
					reactor_cb_args_t args;
					args.type = REACTOR_ERROR;
					args.data.error_args.listening_fd = p_event->data.fd;
					args.data.error_args.p_userdata = inp_self->p_userdata;
					(inp_self->p_cb)(&args);
				}
				close(p_event->data.fd);
				continue;
			}
			else {
				reactor_epoll_accept(inp_self, p_event->data.fd);
			}
		}
		// Look for activity on connections
		n = epoll_wait(inp_self->epoll_event_fd,
				inp_self->p_events, 
				inp_self->num_of_events, in_timeout);
		for(int i = 0; i < n; i++) {
			if(inp_self->p_cb) {
				reactor_cb_args_t args;
				args.type = REACTOR_EVENT;
				args.data.event_args.accepted_fd;
				args.data.event_args.event = inp_self->p_events[i].events;
				args.data.event_args.p_userdata = inp_self->p_events[i].data.ptr;
				(inp_self->p_cb)(&args);
			}
		}
	}
	return 0;
}

int
reactor_loop_once(reactor_pt inp_self) 
{
	return reactor_loop_once_for(inp_self, 0);
}

int
reactor_react_to(reactor_pt inp_self, int listener_fd)
{
	if(inp_self && inp_self->epoll_fd > 0) {
		struct epoll_event event;	
		event.data.fd = listener_fd;
		event.events = EPOLLIN;
		return epoll_ctl(inp_self->epoll_fd, EPOLL_CTL_ADD, listener_fd, &event);
	}
	return -1;
}

int
reactor_unreact_to(reactor_pt inp_self, int listener_fd)
{
	if(inp_self && inp_self->epoll_fd > 0) {
		return epoll_ctl(inp_self->epoll_fd, EPOLL_CTL_DEL, listener_fd, NULL);
	}
	return -1;
}

reactor_pt
reactor_set_userdata(reactor_pt inp_self, void *inp)
{
	if(inp_self) {
		inp_self->p_userdata = inp;
	}
	return inp_self;
}

reactor_pt
reactor_set_cb(reactor_pt inp_self, reactor_cb inp_cb)
{
	if(inp_self) {
		inp_self->p_cb = inp_cb;
	}
	return inp_self;
}

reactor_pt
reactor_set_event_flags(reactor_pt inp_self, uint32_t in_flags)
{
	if(inp_self) {
		inp_self->event_flags = in_flags;
	}
	return inp_self;
}


/*********************************************************************
The following code runs in a seperate thread of execution. It handles
the listening socket io for new incoming connections, accepts them when
they arrive and created a new epoll fd for the newly arriving connection
and adds it to the reactor epoll event system. Also, it listens for an
eventfd change to non-zero to indicate that it should terminate the thd.
**********************************************************************/

static int
reactor_listener_accept(
	int in_listening_fd, 
	int out_fd, 
	int out_nn,
	void *inp_userdata)
{
	char log[256];
	int new_fd;
	socklen_t in_len;
	struct sockaddr in_addr;
	while((new_fd = accept(in_listening_fd, &in_addr, &in_len)) != -1) {
		struct epoll_event event;
		int rc = fcntl(new_fd, F_GETFL, 0);
		if(rc < 0) {
			close(new_fd);
			continue;
		}
		rc |= O_NONBLOCK;
		fcntl(new_fd, F_SETFL, rc);
		event.data.ptr = inp_userdata;
		event.events = 0; 
		rc = epoll_ctl(out_fd, EPOLL_CTL_ADD, new_fd, &event);
		if(out_nn > 0) {
			int len = 0;
			char *p_log = NULL;
			time_t raw = time (&raw);
			struct tm *info = localtime (&raw);
			char *datetext = asctime (info);
			len = snprintf(NULL, 0, "log|[%s] New connection from foo", datetext);
			p_log = calloc(1, len + 1);
			snprintf(p_log, len - 1, "log|[%s] New connection from foo", datetext);
			nn_send(out_nn, log, len, NN_DONTWAIT);
			free(p_log);
		}
        }
        return 0;
}

static int
reactor_listener_thdfunc(
	int in_listen_to_fd, // The Socker IO fd to listen for new connections
	int in_shutdown_fd,  // FD goes non-zero then I shutdown this thread
	int out_fd,          // Add new FD for new connection to this epoll group
	void *inp_userdata)
{
	int fd, nn_sock, stay_alive = 1;

	if((nn_sock = nn_socket(AF_SP, NN_PUB)) > 0) {
		nn_bind(nn_sock, "ipc:///tmp/reactor_listener_logger");
	}

	if(nn_sock < 1) {
		return -1;
	}

	if((fd = epoll_create1(0)) == -1) {
		return 0;
	}

	{	// Add listener FD to my epoll 
		struct epoll_event event;
                event.data.fd = in_listen_to_fd;
                event.events = EPOLLIN;
                epoll_ctl(fd, EPOLL_CTL_ADD, in_listen_to_fd, &event);
	}
	{	// Add shutdown FD to my epoll 
		struct epoll_event event;
                event.data.fd = in_shutdown_fd;
                event.events = EPOLLIN;
                epoll_ctl(fd, EPOLL_CTL_ADD, in_shutdown_fd, &event);
	}

	while(stay_alive) {
		struct epoll_event events[2];
		int n = epoll_wait(fd, events, 2, 1);
		for(int i = 0; i < n; i++) {
			if(events[i].data.fd == in_shutdown_fd) {
				stay_alive = 0;
				break;
			}
			else if(events[i].data.fd == in_listen_to_fd) {
				// Handle new connection.
				reactor_listener_accept(
					in_listen_to_fd,
					out_fd,
					nn_sock,
					inp_userdata);
			}
		}
	}

	if(nn_sock > 0) {
		//nn_shutdown(nn_sock);
	}

	close(fd);
	return 0;
}


#ifdef __cplusplus
}
#endif

