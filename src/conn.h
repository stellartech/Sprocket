


#ifndef CONN_H_INCLUDED
#define CONN_H_INCLUDED

#include <stdint.h>

#include <jansson.h>
#include <pthread.h>
#include <sys/epoll.h>

#ifdef __cplusplus
extern C {
#endif

struct _conn;
typedef struct _conn   conn_t;
typedef struct _conn * conn_pt;

#ifdef CONN_FRIEND_OF
struct _conn
{
	int sock_fd;
	int timer_fd;
	int epoll_fd;
	const char *p_host;
	const char *p_name;
	json_t *p_headers;
	int in_addr_len;
	struct sockaddr in_addr;
	uint64_t fd_bytes_in;
	uint64_t fd_bytes_out;
	epoll_data_t epoll_data;
	void *p_userdata;
	pthread_mutex_t lock;
};
#endif

conn_pt
conn_ctor(int in_epoll_fd, int in_fd, struct sockaddr *inp_addr, int in_addr_len, void *inp_userdata);

void
conn_free(void *inp_self);

void
conn_dtor(conn_pt *inpp_self);


#ifdef __cplusplus
}
#endif

#endif /* CONN_H_INCLUDED */

