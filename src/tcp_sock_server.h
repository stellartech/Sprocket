




#ifndef TCP_SOCK_H_INCLUDED
#define TCP_SOCK_H_INCLUDED

#include <arpa/inet.h>

#ifdef __cplusplus
extern C {
#endif

struct _tcp_sock_server;
typedef struct _tcp_sock_server   tcp_sock_server_t;
typedef struct _tcp_sock_server * tcp_sock_server_pt;
  
typedef void (*p_tcp_sock_server_cb)(
	tcp_sock_server_pt inp_tcp_sock, 
	int fd,
	struct sockaddr *inp_sockaddr,
	int in_sockaddr_len,
	void* inp_userdata
);

#ifdef TCP_SOCK_SERVER_FRIEND 
struct _tcp_sock_server
{
        //! Listening socket file descriptor
        int fd;
	//! Epoll event system fd
	int epoll_fd;
        //! Socket domain (AF_NET or AF_NET6)
        int domain;
        //! The tcp/ip port to listen on.
        short port;
        //! Socket options
        int sock_opts;
        //! Listener backlog
        int backlog;
        //! The raw "human readable" IP address
        char ip_str[INET6_ADDRSTRLEN + 1];
        //! Underlying socket data structure.
        struct sockaddr_in6 in6_addr_buf;
        //! Socket accept callback.
        p_tcp_sock_server_cb p_acc_cb;
        //! Socket error callback;
        p_tcp_sock_server_cb p_err_cb;
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

int
tcp_sock_server_accept(tcp_sock_server_pt inp_self, 
	int in_epoll_fd);

tcp_sock_server_pt
tcp_sock_server_set_epoll_fd(tcp_sock_server_pt inp_self,
	int in_efd);

tcp_sock_server_pt
tcp_sock_server_set_port(tcp_sock_server_pt inp_self, 
	short in_port);

tcp_sock_server_pt
tcp_sock_server_set_acc_cb(tcp_sock_server_pt inp_self, 
	p_tcp_sock_server_cb inp_ptr);

tcp_sock_server_pt
tcp_sock_server_set_err_cb(tcp_sock_server_pt inp_self, 
	p_tcp_sock_server_cb inp_ptr);

tcp_sock_server_pt
tcp_sock_server_set_ip(tcp_sock_server_pt inp_self, 
	const char *inp_ip);

tcp_sock_server_pt
tcp_sock_server_set_userdata(tcp_sock_server_pt inp_self, 
	void *inp_userdata);

tcp_sock_server_pt
tcp_sock_server_conf(tcp_sock_server_pt inp_self, 
	const char *inp_conf_filename);

#ifdef __cplusplus
}
#endif

#endif /* TCP_SOCK_H_INCLUDED */

