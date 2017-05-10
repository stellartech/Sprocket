




#ifndef IPADDR_H_INCLUDED
#define IPADDR_H_INCLUDED

#include <arpa/inet.h>

#ifdef __cplusplus
extern C {
#endif

#ifdef IPADDR_FRIEND_OF
typedef struct
{
	int             domain;
	short           port;
	char            ip_str[INET6_ADDRSTRLEN];
	struct in6_addr in6_addr_buf;

} ipaddr_t, *ipaddr_pt;
#endif /* IPADDR_FRIEND_OF */

ipaddr_pt
ipaddr_ctor(char *inp_ipaddr, short in_port);

void
ipaddr_free(void *inp_self);

void
ipaddr_dtor(ipaddr_pt *inpp_self);

short
ipaddr_get_port(ipaddr_pt inp_self);

int
ipaddr_get_domain(ipaddr_pt inp_self);

const char*
ipaddr_get_ip_str(ipaddr_pt inp_self);

const struct in6_addr*
ipaddr_get_inaddr(ipaddr_pt inp_self);

#ifdef __cplusplus
}
#endif

#endif /* IPADDR_H_INCLUDED */

