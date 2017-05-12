
// ToDo next 
// sort out the callbacks and build an excho POC spike

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>

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
		pthread_mutex_init(&p_self->lock, NULL);
		p_self->refcount = 1;
		p_self->sock_fd = inp_args->in_fd;
		p_self->epoll_fd = inp_args->epoll_fd;
		p_self->close_fd = 0;
		p_self->in_addr_len = inp_args->in_addr_len;
		p_self->p_userdata = inp_args->p_userdata;
		p_self->p_read_cb = inp_args->p_read_cb;
		p_self->p_write_cb = inp_args->p_write_cb;
		p_self->p_error_cb = inp_args->p_error_cb;
		p_self->p_close_cb = inp_args->p_close_cb;
		p_self->conn_state = eSERVER_CONN_OPEN;
		server_conn_set_fdstr(p_self);
		if(inp_args->in_addr_len) {
			memcpy(&p_self->in_addr, inp_args->inp_addr, inp_args->in_addr_len);
		}
		// Lock me.
		server_conn_lock(p_self);
		// Create a timer epoll event.
		if(inp_args->epoll_fd > 0) {
			struct epoll_event d;
			d.data.ptr = p_self;
			d.events = EPOLLIN | EPOLLOUT | EPOLLHUP;
			epoll_ctl(inp_args->epoll_fd, EPOLL_CTL_ADD, inp_args->in_fd, &d);
		}
		// Unlock me.
		server_conn_unlock(p_self);
	}
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
		if(p_self->epoll_fd && p_self->timer_fd) {
			epoll_ctl(p_self->epoll_fd, EPOLL_CTL_DEL,
				p_self->timer_fd, NULL);
		}
		if(p_self->timer_fd) close(p_self->timer_fd);
		if(p_self->epoll_fd && p_self->sock_fd) {
			epoll_ctl(p_self->epoll_fd, EPOLL_CTL_DEL,
				p_self->sock_fd, NULL);
		}
		if(p_self->sock_fd) close(p_self->sock_fd);
		if(p_self->p_name) free((void*)p_self->p_name);
		if(p_self->p_host) free((void*)p_self->p_host);
		if(p_self->p_fdstr) free((void*)p_self->p_fdstr);
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
	// Parent owns the connection so must delete it.
	// This flags this connection should xfer no more
	// data and the parent should just close it.
	if(inp_self) {
		inp_self->conn_state = eSERVER_CONN_CLOSING;
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

const char*
server_conn_name(server_conn_pt inp_self, char *inp_name) 
{
	if(inp_self) {
		if(inp_name) {
			if(inp_self->p_name) free((void*)inp_self->p_name);
			inp_self->p_name = strndup(inp_name, 128);
		}
		return inp_self->p_name;
	}
	return NULL;
}


const char*
server_conn_host(server_conn_pt inp_self, char *inp_host)
{
	if(inp_self) {
		if(inp_host) {
			if(inp_self->p_host) free((void*)inp_self->p_host);
			inp_self->p_host = strndup(inp_host, 128);
		}
		return inp_self->p_host;
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
		int i = snprintf(NULL, 0, "%016x", inp_self->sock_fd);
		if(i > 0) {
			if((inp_self->p_fdstr = calloc(1, i + 1)) != NULL) {
				snprintf((char*)inp_self->p_fdstr, i+1, "%016x", inp_self->sock_fd);
				return inp_self->p_fdstr;
			}
		}
	}
	return NULL;
}
#ifdef __cplusplus
}
#endif

