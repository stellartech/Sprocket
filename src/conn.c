

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>

#define FRIEND_OF_CONN
#include "conn.h"

#ifdef __cplusplus
extern C {
#endif

conn_pt
//conn_ctor(int in_epoll_fd, int in_fd, struct sockaddr *inp_addr, int in_addr_len, void *inp_userdata)
conn_ctor(conn_ctor_args_pt inp_args)
{
	conn_pt p_self = calloc(1, sizeof(conn_t));
	if(p_self) {
		int msec = 1000; // milliseconds
		struct epoll_event d;
		struct itimerspec ts;
		// Boilerplate init code.
		p_self->refcount = 1;
		p_self->sock_fd = inp_args->in_fd;
		p_self->epoll_fd = inp_args->in_epoll_fd;
		p_self->in_addr_len = inp_args->in_addr_len;
		p_self->p_userdata = inp_args->p_userdata;
		p_self->p_read_cb = inp_args->p_read_cb;
		p_self->p_write_cb = inp_args->p_write_cb;
		p_self->p_error_cb = inp_args->p_error_cb;
		p_self->p_close_cb = inp_args->p_close_cb;
		if(inp_args->in_addr_len) {
			memcpy(&p_self->in_addr, inp_args->inp_addr, inp_args->in_addr_len);
		}
		// Lock me.
		pthread_mutex_init(&p_self->lock, NULL);
		conn_lock(p_self);
		//pthread_mutex_lock(&p_self->lock);
		// Create a timer epoll event.
		ts.it_interval.tv_sec = msec / 1000;
		ts.it_interval.tv_nsec = (msec % 1000) * 1000000UL;
		ts.it_value.tv_sec = ts.it_interval.tv_sec;
		ts.it_value.tv_nsec = ts.it_interval.tv_nsec;
		p_self->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
		timerfd_settime(p_self->timer_fd, 0, &ts, NULL);
		d.events = EPOLLIN;
		epoll_ctl(p_self->epoll_fd, EPOLL_CTL_ADD, p_self->timer_fd, &d);
		// Create a socket fd epoll event.
		d.data.ptr = p_self;
		d.events = EPOLLIN | EPOLLOUT | EPOLLHUP;
		epoll_ctl(p_self->epoll_fd, EPOLL_CTL_ADD, inp_args->in_fd, &d);
		// Unlock me.
		conn_unlock(p_self);
		pthread_mutex_unlock(&p_self->lock);
	}
	return p_self;
}

void
conn_free(void *inp_self)
{
	conn_dtor((conn_pt *)&inp_self);
}

void
conn_dtor(conn_pt *inp_conn)
{
	if(inp_conn) {
		conn_pt p_self = *inp_conn;
		if(!p_self) return;
		__sync_fetch_and_sub(&p_self->refcount, 1);
		if(p_self->refcount > 0) return;
		if(p_self->p_close_cb) (p_self->p_close_cb)(p_self);
		pthread_mutex_lock(&p_self->lock);
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
		if(p_self->p_headers) json_decref(p_self->p_headers);
		pthread_mutex_unlock(&p_self->lock);
		pthread_mutex_destroy(&p_self->lock);
		free(p_self);
		*inp_conn = NULL;
	}
}

conn_pt
conn_copy_byref(conn_pt inp_self)
{
	if(inp_self) {
		__sync_fetch_and_add(&inp_self->refcount, 1);
	}
	return inp_self;
}

void
conn_timer_counter_inc(conn_pt inp_self)
{
	if(inp_self) {
		__sync_fetch_and_add(&inp_self->timer_counter, 1);
	}
}

int
conn_get_timer_counter(conn_pt inp_self)
{
	int rval;
	if(inp_self) {
		rval = __sync_fetch_and_add(&inp_self->timer_counter, 0);
	}
	return rval;
}

conn_pt
conn_set_read_cb(conn_pt inp_self, conn_read_cb inp_cb)
{
	if(inp_self) inp_self->p_read_cb = inp_cb;
	return inp_self;
}

conn_pt
conn_set_write_cb(conn_pt inp_self, conn_write_cb inp_cb)
{
	if(inp_self) inp_self->p_write_cb = inp_cb;
	return inp_self;
}

conn_pt
conn_set_error_cb(conn_pt inp_self, conn_error_cb inp_cb)
{
	if(inp_self) inp_self->p_error_cb = inp_cb;
	return inp_self;
}

conn_pt
conn_set_close_cb(conn_pt inp_self, conn_close_cb inp_cb)
{
	if(inp_self) inp_self->p_close_cb = inp_cb;
	return inp_self;
}

int
conn_lock(conn_pt inp_self)
{
	if(inp_self) {
		return pthread_mutex_lock(&inp_self->lock);
	}
	return -1;
}

int
conn_trylock(conn_pt inp_self)
{
	if(inp_self) {
		return pthread_mutex_trylock(&inp_self->lock);
	}
	return -1;
}

int
conn_unlock(conn_pt inp_self)
{
	if(inp_self) {
		return pthread_mutex_unlock(&inp_self->lock);
	}
}

#ifdef __cplusplus
}
#endif

