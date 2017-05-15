




#ifndef TCP_SOCK_H_INCLUDED
#define TCP_SOCK_H_INCLUDED

#include <arpa/inet.h>

#include "../llist.h"
#include "../hashmap.h"

#ifdef __cplusplus
extern C {
#endif

struct _tcp_sock_server;
typedef struct _tcp_sock_server   tcp_sock_server_t;
typedef struct _tcp_sock_server * tcp_sock_server_pt;
  
// Callback function pointer definition.
typedef void (*p_tcp_sock_server_cb)(
	tcp_sock_server_pt inp_tcp_sock, 
	int fd,
	struct sockaddr *inp_sockaddr,
	int in_sockaddr_len,
	void* inp_userdata
);

#define FRIEND_OF_SERVER_CONN
#include "server_conn.h"

// Read callback function pointer prototype.
typedef int (*tcp_sock_server_read_cb)(
	tcp_sock_server_pt p_sock_server,
	server_conn_pt p_server_conn
);
// Write callback function pointer prototype.
typedef int (*tcp_sock_server_write_cb)(
	tcp_sock_server_pt p_sock_server,
	server_conn_pt p_server_conn
);
// Error callback function pointer prototype.
typedef int (*tcp_sock_server_error_cb)(
	tcp_sock_server_pt p_sock_server,
	server_conn_pt p_server_conn
);
// Pulse callback function pointer prototype.
typedef int (*tcp_sock_server_pulse_cb)(
	tcp_sock_server_pt p_sock_server,
	server_conn_pt p_server_conn
);
// Accept callback function pointer prototype.
typedef int (*tcp_sock_server_accept_cb)(
	tcp_sock_server_pt p_sock_server,
	int fd,
	struct sockaddr *p_addr,
	int add_len
);
// Closing callback function pointer prototype.
typedef int (*tcp_sock_server_closing_cb)(
	tcp_sock_server_pt p_sock_server,
	server_conn_pt p_server_conn
);

// Group of named callbacks, name is required.
typedef struct
{
	char p_name[32]; // This is used as the llist key.
	tcp_sock_server_read_cb p_read_cb;
	tcp_sock_server_write_cb p_write_cb;
	tcp_sock_server_error_cb p_error_cb;
	tcp_sock_server_pulse_cb p_pulse_cb;
	tcp_sock_server_accept_cb p_accept_cb;
	tcp_sock_server_closing_cb p_closing_cb;
} tcp_sock_server_callback_t, *tcp_sock_server_callback_pt;

#ifdef TCP_SOCK_SERVER_FRIEND 
struct _tcp_sock_server
{
        //! Listening socket file descriptor
        int fd;
	//! Epoll event system fd
	int epoll_sock_fd;
	//! Epoll event for timers
	int epoll_timer_fd;
        //! Socket domain (AF_NET or AF_NET6)
        int domain;
        //! The tcp/ip port to listen on.
        short port;
        //! Socket options
        int sock_opts;
        //! Listener backlog
        int backlog;
	//! Process maxevents per loop
	int maxevents;
        //! The raw "human readable" IP address
        char ip_str[INET6_ADDRSTRLEN + 1];
        //! Underlying socket data structure.
        struct sockaddr_in6 in6_addr_buf;
	//! List of callback listeners
	llist_pt p_list_listeners;
	//! List of connections.
	hashmap_pt p_hash_conns;
        //! Opaque user data pointer
        void *p_userdata;
        //! User data int
        int userdata_len;
};
#endif

tcp_sock_server_pt
tcp_sock_server_ctor(const char *inp_conf_filename);

void
tcp_sock_server_free(void *inp_self);

void
tcp_sock_server_dtor(tcp_sock_server_pt *inpp_self);

int
tcp_sock_server_get_fd(tcp_sock_server_pt inp_self);

int
tcp_sock_server_bind(tcp_sock_server_pt inp_self);

tcp_sock_server_pt
tcp_sock_server_set_epoll_fd(tcp_sock_server_pt inp_self,
	int in_efd);

tcp_sock_server_pt
tcp_sock_server_set_port(tcp_sock_server_pt inp_self, 
	short in_port);

tcp_sock_server_pt
tcp_sock_server_set_ip(tcp_sock_server_pt inp_self, 
	const char *inp_ip);

tcp_sock_server_pt
tcp_sock_server_set_userdata(tcp_sock_server_pt inp_self, 
	void *inp_userdata);

tcp_sock_server_pt
tcp_sock_server_conf(tcp_sock_server_pt inp_self, 
	const char *inp_conf_filename);

int
tcp_sock_server_add_listener(tcp_sock_server_pt inp_self,
        tcp_sock_server_callback_pt inp_cb);

#ifdef __cplusplus
}
#endif

#endif /* TCP_SOCK_H_INCLUDED */

