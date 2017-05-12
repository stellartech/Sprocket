

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

#include "server_conn.h"

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
		p_self->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
		p_self->p_list_listeners = llist_ctor(NULL);
		p_self->p_hash_conns = hashmap_ctor(1024 * 16, server_conn_free);
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
		LLIST_IF(p_self->p_list_listeners)
			->dtor(&p_self->p_list_listeners);
		if(p_self->fd) {
			close(p_self->fd);
			p_self->fd = 0;
		}
		if(p_self->epoll_fd) {
			close(p_self->epoll_fd);
			p_self->epoll_fd = 0;
		}
		hashmap_dtor(&p_self->p_hash_conns);
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

int
tcp_sock_server_add_listener(tcp_sock_server_pt inp_self, 
	tcp_sock_server_callback_pt inp_cb)
{
	if(inp_self && inp_self->p_list_listeners && inp_cb && inp_cb->p_name) {
		llist_pt p = inp_self->p_list_listeners;
		tcp_sock_server_callback_pt p_cb = calloc(1, sizeof(tcp_sock_server_callback_t));
		if(p_cb) {
			memcpy(p_cb, inp_cb, 
				sizeof(tcp_sock_server_callback_t));
			return LLIST_IF(p)->insert(p, inp_cb->p_name, p_cb);
		}
	}
	return 0;
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

//////////////////
/// Processing
//////////////////

static void
tcp_sock_server_invoke_err_cb(tcp_sock_server_pt inp_self)
{
	if(inp_self) {
		int rc = 1;
		llist_pt p = inp_self->p_list_listeners;
		if(LLIST_IF(p)->count(p) == 0) return;
		llist_iterator_pt p_iter = LLIST_IF(p)->iterator_new(p);
		tcp_sock_server_callback_pt p_val = LLIST_IF(p)->iterator_current(p_iter);
		while(rc && p_val) {
			if(p_val->p_error_cb) {
				(p_val->p_error_cb)(inp_self, NULL);
			}
			rc = LLIST_IF(p)->iterator_forward(p_iter);
		}
		LLIST_IF(p)->iterator_free(p_iter);
	}
}

static int
tcp_sock_server_accept(tcp_sock_server_pt inp_self)
{
	int fd, addr_len;
	struct sockaddr addr;
	if(!inp_self) return -1;
	while(1) {
		addr_len = 0;
		memset(&addr, 0, sizeof(struct sockaddr));
		fd = accept(inp_self->epoll_fd, &addr, &addr_len);
		if(fd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			// All incoming connections processed.
			return 0;
		}
		else if(fd == -1) {
			tcp_sock_server_invoke_err_cb(inp_self);
			return 0;
		}
		else {
			server_conn_pt p_server;
			server_conn_ctor_args_t args;

			memset(&args, 0, sizeof(server_conn_ctor_args_t));
		        args.in_fd = fd;
			args.epoll_fd = inp_self->epoll_fd;
			args.inp_addr = &addr;
        		args.in_addr_len = addr_len;
        		args.p_userdata = NULL;
			p_server = server_conn_ctor(&args);
			if(!p_server) {
				close(fd);
				// Error!
				continue;
			}
			// Note, the hashmap takes ownership of the p_server pointer.
			// Deleting it from the hashmap will invoke server_conn_free()
			// on the pointer automatically. Destruction of the hashmap with
			// it's dtor will do the same. The correct way to close a conn
			// is to simply delete it from the hashmap.
			// Hashmap "remove" allows to extract the p_server pointer without
			// invoking the destructor. You are basically taking ownsership by
			// doing this, now down to you to destroy it with dtor/free.
			hashmap_insert(inp_self->p_hash_conns, p_server->p_fdstr, p_server);

			{ // Accept callbacks
				int rc = 0;
				llist_pt p = inp_self->p_list_listeners;
				rc = LLIST_IF(p)->count(p);
				llist_iterator_pt p_iter = LLIST_IF(p)->iterator_new(p);
				while(rc) {
					tcp_sock_server_callback_pt p_val = LLIST_IF(p)->iterator_current(p_iter);
					if(p_val->p_accept_cb) {
						if((p_val->p_accept_cb)(inp_self, fd, &addr, addr_len) < 0) {
							close(fd);
							break;
						}
					}
					rc = LLIST_IF(p)->iterator_forward(p_iter);
				}
				LLIST_IF(p)->iterator_free(p_iter);
			}
		}
	}
	return 0;
}

#ifdef __cplusplus
}
#endif


