


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

typedef void (*conn_read_cb)(conn_pt p_conn, struct epoll_event *p_event);
typedef void (*conn_write_cb)(conn_pt p_conn, struct epoll_event *p_event);
typedef void (*conn_error_cb)(conn_pt p_conn);
typedef void (*conn_close_cb)(conn_pt p_conn);

#ifdef FRIEND_OF_CONN
struct _conn
{
	int sock_fd;
	int timer_fd;
	int epoll_fd;
	int refcount;
	uint64_t timer_counter;
	const char *p_host;
	const char *p_name;
	json_t *p_headers;
	int in_addr_len;
	struct sockaddr in_addr;
	uint64_t fd_bytes_in;
	uint64_t fd_bytes_out;
	epoll_data_t epoll_data;
	conn_read_cb p_read_cb;
	conn_write_cb p_write_cb;
	conn_error_cb p_error_cb;
	conn_close_cb p_close_cb;
	void *p_userdata;
	pthread_mutex_t lock;
};
#endif

typedef struct
{
	int in_epoll_fd;
	int in_fd;
	struct sockaddr *inp_addr;
	int in_addr_len;
	conn_read_cb p_read_cb;
	conn_write_cb p_write_cb;
	conn_error_cb p_error_cb;
	conn_close_cb p_close_cb;
	void *p_userdata;
} conn_ctor_args_t, *conn_ctor_args_pt;

//conn_pt
//conn_ctor(int in_epoll_fd, int in_fd, struct sockaddr *inp_addr, int in_addr_len, void *inp_userdata);
conn_pt
conn_ctor(conn_ctor_args_pt inp_args);

void
conn_free(void *inp_self);

void
conn_dtor(conn_pt *inpp_self);

/**
 * Returns the same pointer and increments the refcount.
 * Thread safe, uses gcc atomic operations.
 */
conn_pt
conn_copy_byref(conn_pt inp_self);

/**
 * Increments the internal timer counter.
 * Thread safe, uses gcc atomic operations.
 */
void
conn_timer_counter_inc(conn_pt inp_self);

/**
 * Gets the current value of the timer counter.
 * Thread safe, uses gcc atomic operations.
 */
int
conn_get_timer_counter(conn_pt inp_self);

/////////////////////////////////////////////////////////////////
/// Thread locking functions:
/// @return 0 on success, -1 on p_self invalid or an E number:
/// https://linux.die.net/man/3/pthread_mutex_lock
/////////////////////////////////////////////////////////////////
int
conn_lock(conn_pt inp_self);

int
conn_trylock(conn_pt inp_self);

int
conn_unlock(conn_pt inp_self);

/////////////////////////////////////////////////////////////////
/// Set callback functions.
/////////////////////////////////////////////////////////////////
conn_pt
conn_set_read_cb(conn_pt inp_self, conn_read_cb inp_cb);

conn_pt
conn_set_write_cb(conn_pt inp_self, conn_write_cb inp_cb);

conn_pt
conn_set_error_cb(conn_pt inp_self, conn_error_cb inp_cb);

conn_pt
conn_set_close_cb(conn_pt inp_self, conn_close_cb inp_cb);

#ifdef __cplusplus
}
#endif

#endif /* CONN_H_INCLUDED */

