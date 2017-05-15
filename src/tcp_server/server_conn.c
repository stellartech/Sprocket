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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>

#define FRIEND_OF_SERVER_CONN
#include "server_conn.h"

#ifdef __cplusplus
extern C {
#endif

static const char*
server_conn_set_fdstr(server_conn_pt inp_self);

server_conn_pt
server_conn_ctor(server_conn_ctor_args_pt inp_args)
{
	server_conn_pt p_self = calloc(1, sizeof(server_conn_t));
	if(p_self) {
		// Boilerplate init code.
		if(pthread_mutex_init(&p_self->lock, NULL) != 0) {
			free(p_self);
			return NULL;
		}
		server_conn_lock(p_self);
		p_self->refcount = 1;
		p_self->epoll_sock_fd = inp_args->epoll_sock_fd;
		p_self->epoll_timer_fd = inp_args->epoll_timer_fd;
		p_self->in_addr_len = inp_args->in_addr_len;
		p_self->p_userdata = inp_args->p_userdata;
		p_self->p_read_cb = inp_args->p_read_cb;
		p_self->p_write_cb = inp_args->p_write_cb;
		p_self->p_error_cb = inp_args->p_error_cb;
		p_self->p_close_cb = inp_args->p_close_cb;
		p_self->conn_state = eSERVER_CONN_OPEN;
		p_self->p_userdata = NULL;
		p_self->sock_fd = inp_args->in_fd;
		if(p_self->sock_fd > 0) { 
			int flags;
			if((flags = fcntl(p_self->sock_fd, F_GETFL, 0)) == -1)
				goto server_conn_ctor_failed;
			flags |= O_NONBLOCK;
			if((fcntl(p_self->sock_fd, F_SETFL, flags)) == -1)
				goto server_conn_ctor_failed;
		}
		server_conn_set_fdstr(p_self);
		if(inp_args->in_addr_len) {
			memcpy(&p_self->in_addr, inp_args->inp_addr, inp_args->in_addr_len);
		}
		// Create a timer epoll event and a close signal event.
		if(inp_args->epoll_sock_fd > 0) {
			struct epoll_event d;
			d.data.ptr = p_self;
			d.events = EPOLLIN | EPOLLOUT | EPOLLHUP;
			if((epoll_ctl(inp_args->epoll_sock_fd, EPOLL_CTL_ADD, inp_args->in_fd, &d)) == -1)
				goto server_conn_ctor_failed;
		}
		else {
			goto server_conn_ctor_failed;
		}
		if(inp_args->p_open_cb != NULL) {
			if((inp_args->p_open_cb)(p_self) != 0) {
				goto server_conn_ctor_failed;
			}
		}
		server_conn_unlock(p_self);
	}
	return p_self;

	server_conn_ctor_failed:
	if(p_self) p_self->sock_fd = 0;
	server_conn_unlock(p_self);
	server_conn_dtor(&p_self);
	return p_self;
}

void
server_conn_free(void *inp_self)
{
	server_conn_dtor((server_conn_pt *)&inp_self);
}

void
server_conn_dtor(server_conn_pt *inp_server_conn)
{
	if(inp_server_conn) {
		server_conn_pt p_self = *inp_server_conn;
		if(!p_self) return;
		server_conn_lock(p_self);
		p_self->refcount = __sync_fetch_and_sub(&p_self->refcount, 1);
		if(p_self->refcount > 0) {
			server_conn_unlock(p_self);
			return;
		}
		if(p_self->p_close_cb) (p_self->p_close_cb)(p_self);
		if(p_self->epoll_timer_fd && p_self->timer_fd) {
			epoll_ctl(p_self->epoll_timer_fd, EPOLL_CTL_DEL,
				p_self->timer_fd, NULL);
		}
		if(p_self->timer_fd) close(p_self->timer_fd);
		if(p_self->epoll_sock_fd && p_self->sock_fd) {
			epoll_ctl(p_self->epoll_sock_fd, EPOLL_CTL_DEL,
				p_self->sock_fd, NULL);
		}
		if(p_self->sock_fd) close(p_self->sock_fd);
		if(p_self->close_fd) close(p_self->sock_fd);
		server_conn_unlock(p_self);
		pthread_mutex_destroy(&p_self->lock);
		free(p_self);
		*inp_server_conn = NULL;
	}
}

server_conn_pt
server_conn_copy_byref(server_conn_pt inp_self)
{
	if(inp_self) {
		inp_self->refcount = __sync_fetch_and_add(&inp_self->refcount, 1);
	}
	return inp_self;
}

void
server_conn_timer_counter_inc(server_conn_pt inp_self)
{
	if(inp_self) 
		inp_self->timer_counter =
			__sync_fetch_and_add(&inp_self->timer_counter, 1);
}

void
server_conn_close(server_conn_pt inp_self)
{
	if(inp_self) {
		uint64_t sig = 1;
		inp_self->conn_state = eSERVER_CONN_CLOSING;
		write(inp_self->close_fd, &sig, sizeof(uint64_t));
	}
}

int
server_conn_close_requested(server_conn_pt inp_self)
{
	if(inp_self) {
		uint64_t sig = 0;
		read(inp_self->close_fd, &sig, sizeof(uint64_t));
		return sig > 0 ? 1 : 0;
	}
}

int
server_conn_get_timer_counter(server_conn_pt inp_self)
{
	int rval;
	if(inp_self) {
		rval = __sync_fetch_and_add(&inp_self->timer_counter, 0);
	}
	return rval;
}

void
server_conn_set_timer(server_conn_pt inp_self, int in_efd, struct itimerspec *inp_ts)
{
	if(inp_self) {
		struct epoll_event d;
		inp_self->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
		timerfd_settime(inp_self->timer_fd, 0, inp_ts, NULL);
		d.events = EPOLLIN;
		d.data.ptr = inp_self;
		epoll_ctl(in_efd, EPOLL_CTL_ADD, inp_self->timer_fd, &d);
	}
}

int
server_conn_get_sock_fd(server_conn_pt inp_self)
{
	if(inp_self) {
		return inp_self->sock_fd;
	}
	return 0;
}

const char*
server_conn_get_sock_fdstr(server_conn_pt inp_self)
{
	if(inp_self) {
		return &inp_self->fdstr[0];
	}
	return NULL;
}

server_conn_pt
server_conn_set_read_cb(server_conn_pt inp_self, server_conn_read_cb inp_cb)
{
	if(inp_self) inp_self->p_read_cb = inp_cb;
	return inp_self;
}

server_conn_pt
server_conn_set_write_cb(server_conn_pt inp_self, server_conn_write_cb inp_cb)
{
	if(inp_self) inp_self->p_write_cb = inp_cb;
	return inp_self;
}

server_conn_pt
server_conn_set_error_cb(server_conn_pt inp_self, server_conn_error_cb inp_cb)
{
	if(inp_self) inp_self->p_error_cb = inp_cb;
	return inp_self;
}

server_conn_pt
server_conn_set_close_cb(server_conn_pt inp_self, server_conn_close_cb inp_cb)
{
	if(inp_self) inp_self->p_close_cb = inp_cb;
	return inp_self;
}

int
server_conn_lock(server_conn_pt inp_self)
{
	if(inp_self) {
		return pthread_mutex_lock(&inp_self->lock);
	}
	return -1;
}

int
server_conn_trylock(server_conn_pt inp_self)
{
	if(inp_self) {
		return pthread_mutex_trylock(&inp_self->lock);
	}
	return -1;
}

int
server_conn_unlock(server_conn_pt inp_self)
{
	if(inp_self) {
		return pthread_mutex_unlock(&inp_self->lock);
	}
}

static const char*
server_conn_set_fdstr(server_conn_pt inp_self)
{
	if(inp_self) {
		int i = snprintf(NULL, 0, "%016x", inp_self->sock_fd) + 1;
		memset((void*)&inp_self->fdstr[0], 0, sizeof(inp_self->fdstr));
		if(i > 0 && i <= (sizeof(inp_self->fdstr))) {
			snprintf((char*)&inp_self->fdstr[0], i, "%016x", inp_self->sock_fd);
			return &inp_self->fdstr[0];
		}
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

