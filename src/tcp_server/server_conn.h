


#ifndef SERVER_CONN_H_INCLUDED
#define SERVER_CONN_H_INCLUDED

#include <stdint.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern C {
#endif

struct _server_conn;
typedef struct _server_conn   server_conn_t;
typedef struct _server_conn * server_conn_pt;

typedef void (*server_conn_read_cb)(server_conn_pt p_server_conn, struct epoll_event *p_event);
typedef void (*server_conn_write_cb)(server_conn_pt p_server_conn, struct epoll_event *p_event);
typedef void (*server_conn_error_cb)(server_conn_pt p_server_conn);
typedef void (*server_conn_close_cb)(server_conn_pt p_server_conn);

typedef enum {
	eSERVER_CONN_OPEN = 0,
	eSERVER_CONN_CLOSING,
	eSERVER_CONN_CLOSED
} eSERVER_CONN_STATE;

#ifdef FRIEND_OF_SERVER_CONN
struct _server_conn
{
	//! The underlying socket fd for this connection
	int sock_fd;
	//! The underlying timer fd for this connection
	int timer_fd;
	//! The parents central loop epoll fd
	int epoll_fd;
	//! Signal to parent closure required
	int close_fd;
	//! Refcounted object.
	int refcount;
	//! The state of the conn (eSERVER_CONN_STATE
	eSERVER_CONN_STATE conn_state;
	//! Incremented by the timer_fd
	uint64_t timer_counter;
	//! String representation of the sock_fd
	const char *p_fdstr;
	const char *p_host;
	const char *p_name;
	//! The connections caller info
	struct sockaddr in_addr;
	//! The length of the above.
	int in_addr_len;
	//! Metrics counter, bytes in the sock_fd
	uint64_t fd_bytes_in;
	//! Metrics counter, bytes out the sock_fd
	uint64_t fd_bytes_out;
	//!
	epoll_data_t epoll_data;
	//! Event callbacks.
	server_conn_read_cb p_read_cb;
	server_conn_write_cb p_write_cb;
	server_conn_error_cb p_error_cb;
	server_conn_close_cb p_close_cb;
	//! Carry a user's payload around for them.
	void *p_userdata;
	//! The thread lock for this connection.
	pthread_mutex_t lock;
};
#endif

typedef struct
{
	int in_fd;
	int epoll_fd;
	struct sockaddr *inp_addr;
	int in_addr_len;
	server_conn_read_cb p_read_cb;
	server_conn_write_cb p_write_cb;
	server_conn_error_cb p_error_cb;
	server_conn_close_cb p_close_cb;
	void *p_userdata;
} server_conn_ctor_args_t, *server_conn_ctor_args_pt;

#ifdef FRIEND_OF_SERVER_CONN
server_conn_pt
server_conn_ctor(server_conn_ctor_args_pt inp_args);
#endif

#ifdef FRIEND_OF_SERVER_CONN
void
server_conn_free(void *inp_self);
#endif

#ifdef FRIEND_OF_SERVER_CONN
void
server_conn_dtor(server_conn_pt *inpp_self);
#endif

/**
 * Returns the same pointer and increments the refcount.
 * Thread safe, uses gcc atomic operations.
 */
server_conn_pt
server_conn_copy_byref(server_conn_pt inp_self);

/**
 * Increments the internal timer counter.
 * Thread safe, uses gcc atomic operations.
 */
void
server_conn_timer_counter_inc(server_conn_pt inp_self);

/**
 * Set up an epoll timer on this connection.
 */
void
server_conn_set_timer(server_conn_pt inp_self, 
	int in_efd, struct itimerspec *inp_ts);

const char*
server_conn_name(server_conn_pt inp_self,
	char *inp_name);

const char*
server_conn_host(server_conn_pt inp_self,
	char *inp_host);

/**
 * Gets the current value of the timer counter.
 * Thread safe, uses gcc atomic operations.
 */
int
server_conn_get_timer_counter(server_conn_pt inp_self);

/////////////////////////////////////////////////////////////////
/// Thread locking functions:
/// @return 0 on success, -1 on p_self invalid or an E number:
/// https://linux.die.net/man/3/pthread_mutex_lock
/////////////////////////////////////////////////////////////////
int
server_conn_lock(server_conn_pt inp_self);

int
server_conn_trylock(server_conn_pt inp_self);

int
server_conn_unlock(server_conn_pt inp_self);

/////////////////////////////////////////////////////////////////
/// Set callback functions.
/////////////////////////////////////////////////////////////////
server_conn_pt
server_conn_set_read_cb(server_conn_pt inp_self, server_conn_read_cb inp_cb);

server_conn_pt
server_conn_set_write_cb(server_conn_pt inp_self, server_conn_write_cb inp_cb);

server_conn_pt
server_conn_set_error_cb(server_conn_pt inp_self, server_conn_error_cb inp_cb);

server_conn_pt
server_conn_set_close_cb(server_conn_pt inp_self, server_conn_close_cb inp_cb);

void
server_conn_close(server_conn_pt inp_self);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_CONN_H_INCLUDED */

