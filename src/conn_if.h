



#ifndef CONN_IF_INCLUDED
#define CONN_IF_INCLUDED

#include <arpa/inet.h>

#include "utils/iovarr.h"

struct _conn_t;
typedef struct _conn_t   conn_t;
typedef struct _conn_t * conn_pt;

typedef void (*pf_close)(conn_pt);
typedef iovarr_pt (*pf_conn_read)(conn_pt);
typedef int (*pf_conn_write)(conn_pt, void *p_buf, int buflen);

typedef struct 
{
	pf_close	close;
	pf_conn_read	read;	
	pf_conn_write	write;
}
conn_if_t, *conn_if_pt;

struct _conn_t
{
	conn_if_t	iface;
	struct sockaddr	addr;
	socklen_t	addrlen;	
	void		*p_userdata;
};

#endif /* CONN_IF_INCLUDED */

