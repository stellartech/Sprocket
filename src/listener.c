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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "listener.h"

struct _listener
{
	//! The listening socket file descriptor
	int fd;
	//! AF_INET or AF_INET6
	int domain;
	//! Socket options at bind time.
	int sock_opts;
	//! Socket backlog for listen()
	int backlog;
	//! String of user supplied bind addr
	const char ip[INET6_ADDRSTRLEN + 1];
	//! Opening flags.
	unsigned flags; // Any number of LEV_OPT_* flags
	//! Sock struct
	union {
		struct sockaddr_in addr_buf4;
		struct sockaddr_in6 addr_buf6;
	} sock;
	//! Length of addr_buf above.
	int addr_buf_len;
};

int
listener_bind(listener_pt inp_self)
{
	if(inp_self) {
		int fd, on = 1, len = 0, rc = 0;
		if(0 > (fd = socket(inp_self->domain, SOCK_STREAM | SOCK_NONBLOCK, 0))) {
			return -1;
		}
		if(0 > setsockopt(fd, SOL_SOCKET, inp_self->sock_opts, (void*)&on, sizeof(on))) {
			close(fd);
			return -2;
		}
		if(inp_self->domain == AF_INET) {
			len = sizeof(struct sockaddr_in);
			rc = bind(fd, (struct sockaddr *)&inp_self->sock.addr_buf4, len);
		}
		else {
			len = sizeof(struct sockaddr_in6);
			rc = bind(fd, (struct sockaddr *)&inp_self->sock.addr_buf6, len);
		}
		if(rc != 0) {
			close(fd);
			return -3;
		}
		inp_self->fd = fd;
	}
	return inp_self->fd;
}

int
listener_listen(listener_pt inp_self)
{
	if(inp_self && inp_self->fd) {
		return listen(inp_self->fd, inp_self->backlog);
	}
	return -4;
}

listener_pt
listener_set_backlog(listener_pt inp_self, int in_backlog)
{
	if(inp_self) {
		inp_self->backlog = in_backlog;
	}
	return inp_self;
}

listener_pt
listener_ctor(const char *inp_ip, short in_port)
{
	listener_pt p_self = calloc(1, sizeof(listener_t));
	if(p_self) {
		if(inp_ip != NULL && in_port > 0) {
			if(listener_set_ipaddr(p_self, inp_ip, strlen(inp_ip)) != 1) {
				listener_free(p_self);
				return NULL;
			}
			listener_set_port(p_self, in_port);
		}
		p_self->sock_opts = SO_REUSEADDR | 
					SO_REUSEPORT |
					SO_KEEPALIVE;
		p_self->backlog = -1;
	}
	return p_self;
}

void
listener_free(listener_pt inp_self)
{
	if(inp_self) {
		if(inp_self->fd) {
			close(inp_self->fd);
			inp_self->fd = 0;
		}
		free(inp_self);
	}	
}

void
listener_dtor(listener_pt *inpp_self)
{
	if(inpp_self) {
		listener_pt p_self = *inpp_self;
		if(p_self) {
			listener_free(p_self);
		}
		*inpp_self = NULL;
	}
}

listener_pt
listener_set_flags(listener_pt inp_self, unsigned in_flags)
{
	if(inp_self) {
		inp_self->flags = in_flags;
	}
	return inp_self;
}

int
listener_get_domain(listener_pt inp_self)
{
	if(inp_self) {
		return inp_self->domain;
	}
	return -1;
}

static int
count_chars_in_str(const char *inp_str, char in_c, int in_len)
{
	int counter = 0;
	char *s = (char*)inp_str;
	while(*s && in_len > 0) {
		if((*s) == in_c) counter++;
		s++;
		in_len--;
	}
	return counter;
}

int
listener_set_ipaddr(listener_pt inp_self, const char *inp_addr, int in_len)
{
	if(in_len > INET6_ADDRSTRLEN) {
		return -3;
	}
	if(inp_self) {
		int periods;
		memset((void*)inp_self->ip, 0, INET6_ADDRSTRLEN + 1);
		strncpy((char*)inp_self->ip, inp_addr, INET6_ADDRSTRLEN);
		periods = count_chars_in_str(inp_self->ip, '.', in_len);
		inp_self->domain = periods == 3 ? AF_INET : AF_INET6;
		if(inp_self->domain == AF_INET) {
			inp_self->sock.addr_buf4.sin_family = AF_INET;
			return inet_pton(AF_INET, inp_self->ip, &inp_self->sock.addr_buf4.sin_addr);
		}
		inp_self->sock.addr_buf6.sin6_family = AF_INET6;
		return inet_pton(AF_INET6, inp_self->ip, &inp_self->sock.addr_buf6.sin6_addr);
	}
	return -2;

}

listener_pt
listener_set_port(listener_pt inp_self, short in_port)
{
	if(inp_self) {
		if(inp_self->domain == AF_INET) {
			inp_self->sock.addr_buf4.sin_port = htons(in_port);
			return inp_self;
		}
		inp_self->sock.addr_buf6.sin6_port = htons(in_port);
	}
	return inp_self;
}

int
listener_get_fd(listener_pt inp_self) 
{
	if(inp_self) {
		return inp_self->fd;
	}
	return -1;
}

