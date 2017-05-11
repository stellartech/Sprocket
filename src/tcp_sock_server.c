

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define TCP_SOCK_SERVER_FRIEND
#include "tcp_sock_server.h"

#include "conn.h"

#ifdef __cplusplus
extern C {
#endif

tcp_sock_server_pt
tcp_sock_server_ctor(const char *inp_conf_filename)
{
	tcp_sock_server_pt p_self = calloc(1, sizeof(tcp_sock_server_t));
	if(p_self) {
		p_self->sock_opts =
			SO_REUSEADDR | 
			SO_REUSEPORT |
			SO_KEEPALIVE;
		p_self->backlog = 1024;
		p_self->domain = AF_INET;
		p_self->epoll_fd = 0;
		tcp_sock_server_conf(p_self, inp_conf_filename);
	}
	return p_self;

	tcp_sock_server_ctor_fail:
	tcp_sock_server_free(p_self);
	return NULL;
}

void
tcp_sock_server_free(void *inp_self)
{
	tcp_sock_server_pt p_self = (tcp_sock_server_pt)inp_self;
	if(p_self) {
		if(p_self->fd) {
			close(p_self->fd);
			p_self->fd = 0;
		}
		free(inp_self);
	}
}

void
tcp_sock_server_dtor(tcp_sock_server_pt *inpp_self)
{
	if(inpp_self) {
		tcp_sock_server_pt p_self = *inpp_self;
		tcp_sock_server_free((void*)p_self);
		*inpp_self = NULL;
	}
}

int
tcp_sock_server_bind(tcp_sock_server_pt inp_self)
{
	if(inp_self) {
		int i, on = 1, len = 0;
		if(0 > (i = socket(inp_self->domain, SOCK_STREAM | SOCK_NONBLOCK, 0))) {
			return -1;
		}
		if(0 > setsockopt(i, SOL_SOCKET, inp_self->sock_opts, (void*)&on, sizeof(on))) {
			close(i);
			return -2;
		}
		len = (inp_self->domain == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
		if(0 != bind(i, (struct sockaddr *)&inp_self->in6_addr_buf, len)) {
			close(i);
			return -3;
		}
		if(0 > listen(i, inp_self->backlog)) {
			close(i);
			return -4;
		}
		inp_self->fd = i;
	}
	return inp_self->fd;
}

// Compile fails here.
// We need to decide if we are going to run two epolls, one for accepting new conns and one for handling them
int
tcp_sock_server_event(tcp_sock_server_pt inp_self, int in_efd, struct epoll_event *inp_event)
{
	int fd, addr_len;
	struct sockaddr addr;
	if(!inp_self) return -1;
	if(inp_self->epoll_fd != in_efd) return -1;
	while(1) {
		fd = accept(in_efd, &addr, &addr_len);
		if(fd == -1) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				// All incoming connections processed.
				return 0;
			}
			else {
				// Error callback.
				(inp_self->p_err_cb)(inp_self, fd, &addr, addr_len, inp_self->p_userdata);
				return 0;
			}
		}
		else {
			if(inp_self->p_acc_cb) {
				// Callback to accept handler.
				(inp_self->p_acc_cb)(inp_self, fd, &addr, addr_len, inp_self->p_userdata);
			}
			else close(fd);
		}
	}
	return 0;
}

int
tcp_sock_server_get_fd(tcp_sock_server_pt inp_self)
{
	if(inp_self) {
		return inp_self->fd;
	}
	return -1;	
}

tcp_sock_server_pt
tcp_sock_server_set_port(tcp_sock_server_pt inp_self, short in_port)
{
	if(inp_self) {
		inp_self->port = in_port;
		inp_self->in6_addr_buf.sin6_port = htons(inp_self->port);
	}
	return inp_self;
}

tcp_sock_server_pt
tcp_sock_server_set_epoll_fd(tcp_sock_server_pt inp_self, int in_efd)
{
	if(inp_self) {
		inp_self->epoll_fd = in_efd;
	}
	return inp_self;
}

tcp_sock_server_pt
tcp_sock_server_set_acc_cb(tcp_sock_server_pt inp_self, p_tcp_sock_server_cb inp_ptr)
{
	if(inp_self) {
		inp_self->p_acc_cb = inp_ptr;
	}
	return inp_self;
}

tcp_sock_server_pt
tcp_sock_server_set_err_cb(tcp_sock_server_pt inp_self, p_tcp_sock_server_cb inp_ptr)
{
	if(inp_self) {
		inp_self->p_err_cb = inp_ptr;
	}
	return inp_self;
}

tcp_sock_server_pt
tcp_sock_server_set_ip(tcp_sock_server_pt inp_self, const char *inp_ip)
{
	if(inp_self) {
		strncpy(inp_self->ip_str, inp_ip, INET6_ADDRSTRLEN);
		inp_self->domain = (strchr(inp_self->ip_str, ':') == NULL) ? AF_INET : AF_INET6;
		if(0 >= (inet_pton(inp_self->domain, inp_self->ip_str, &inp_self->in6_addr_buf))) {
			return NULL;
		}

	}
	return inp_self;
}

tcp_sock_server_pt
tcp_sock_server_set_userdata(tcp_sock_server_pt inp_self, void *inp_userdata)
{
	if(inp_self) {
		inp_self->p_userdata = inp_userdata;
	}
	return inp_self;
}

#include <libconfig.h>

tcp_sock_server_pt
tcp_sock_server_conf(tcp_sock_server_pt inp_self, const char *inp_conf_filename)
{
	if(inp_self && inp_conf_filename) {
		int i;
		const char *p_value;
		config_t config;
		if(config_read_file(&config, inp_conf_filename) != CONFIG_TRUE) return NULL;
		if(config_lookup_string(&config, "ip", &p_value) == CONFIG_TRUE) 
			tcp_sock_server_set_ip(inp_self, p_value);
		if(config_lookup_int(&config, "port", &i) == CONFIG_TRUE)
			tcp_sock_server_set_port(inp_self, (short)(i&0xffff));
		if(config_lookup_int(&config, "opts", &i) == CONFIG_TRUE)
			inp_self->sock_opts = i;
		if(config_lookup_int(&config, "backlog", &i) == CONFIG_TRUE)
			inp_self->backlog =  i;
		config_destroy(&config);
	}
}

#ifdef __cplusplus
}
#endif


