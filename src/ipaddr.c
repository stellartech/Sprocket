


#define IPADDR_FRIEND_OF
#include "ipaddr.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern C {
#endif

ipaddr_pt
ipaddr_ctor(char *inp_ipaddr, short in_port)
{
	ipaddr_pt p_self = calloc(1, sizeof(ipaddr_t));
	if(p_self) {
		p_self->port = in_port;
		if(inp_ipaddr) {
			strncpy(p_self->ip_str, inp_ipaddr, INET6_ADDRSTRLEN);
			p_self->domain = (strchr(p_self->ip_str, ':') == NULL) ? AF_INET : AF_INET6;
			if(0 >= (inet_pton(p_self->domain, p_self->ip_str, &p_self->in6_addr_buf))) {
				goto ipaddr_ctor_fail;
			}
		}
	}
	return p_self;
ipaddr_ctor_fail:
	ipaddr_free(p_self);
	return NULL;
}

void
ipaddr_free(void *inp_self)
{
	if(inp_self) {
		free(inp_self);
	}
}

void
ipaddr_dtor(ipaddr_pt *inpp_self)
{
	if(inpp_self) {
		ipaddr_pt p_self = *inpp_self;
		ipaddr_free(p_self);
		*inpp_self = NULL;
	}
}

short
ipaddr_get_port(ipaddr_pt inp_self)
{
	return inp_self->port;
}

int
ipaddr_get_domain(ipaddr_pt inp_self)
{
	return inp_self->domain;
}

const char*
ipaddr_get_ip_str(ipaddr_pt inp_self)
{
	return inp_self->ip_str;
}

const struct in6_addr*
ipaddr_get_inaddr(ipaddr_pt inp_self)
{
	return (const struct in6_addr*)&inp_self->in6_addr_buf;
}


#ifdef __cplusplus
}
#endif

