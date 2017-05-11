

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <arpa/inet.h>

#define CONN_FRIEND_OF
#include "conn.h"

#ifdef __cplusplus
extern C {
#endif

#if 0
struct _conn
{
	int fd;
	const char *p_host;
	const char *p_name;
	json_t *p_headers;
	int in_addr_len;
	struct sockaddr in_addr;
	uint64_t fd_bytes_in;
	uint64_t fd_bytes_out;
	void *p_userdata;
	pthread_mutexi_t lock;
};
#endif

conn_pt
conn_ctor(int in_epoll_fd, int in_fd, struct sockaddr *inp_addr, int in_addr_len, void *inp_userdata)
{
	conn_pt p_self = calloc(1, sizeof(conn_t));
	if(p_self) {
		struct epoll_event d;
		p_self->sock_fd = in_fd;
		p_self->epoll_fd = in_epoll_fd;
		p_self->in_addr_len = in_addr_len;
		p_self->p_userdata = inp_userdata;
		if(in_addr_len) {
			memcpy(&p_self->in_addr, inp_addr, in_addr_len);
		}
		pthread_mutex_init(&p_self->lock, NULL);
		pthread_mutex_lock(&p_self->lock);
		d.data.ptr = p_self;
		d.events = EPOLLIN | EPOLLOUT | EPOLLHUP;
		epoll_ctl(p_self->epoll_fd, EPOLL_CTL_ADD, in_fd, &d);
		pthread_mutex_unlock(&p_self->lock);
	}
	return p_self;
}

void
conn_free(void *inp_self)
{
	if(inp_self) {
		conn_pt p_self = (conn_pt)inp_self;
		pthread_mutex_lock(&p_self->lock);
		if(p_self->epoll_fd) {
			epoll_ctl(p_self->epoll_fd, EPOLL_CTL_DEL, p_self->fd, NULL);
		}
		if(p_self->fd) close(p_self->fd);
		pthread_mutex_unlock(&p_self->lock);
		pthread_mutex_destroy(&p_self->lock);
		free(inp_self);
	}	
}

void
conn_dtor(conn_pt *inpp_self)
{
	if(inpp_self) {
		conn_free(*inpp_self);
		*inpp_self = NULL;
	}
}


#ifdef __cplusplus
}
#endif

