

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IPADDR_FRIEND_OF
#include "../src/ipaddr.h"

int
main(int argc, char *argv[])
{
	char str[INET6_ADDRSTRLEN];
	ipaddr_pt ip = ipaddr_ctor("0.0.0.0", 443);


	printf("IP: %s:%d\n", ip->ip_str, ip->port);

	inet_ntop(ip->domain, &ip->in6_addr_buf, str, INET6_ADDRSTRLEN);
	printf("IP: %s:%d\n", str, ip->port);

	ipaddr_free(ip);
	return 0;

}

#if 0
#include <stdio.h>

int main(int argc, char** argv) 
{
	printf("Hello World\n");
	return 0;	
}
#endif

