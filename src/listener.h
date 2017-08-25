/*********************************************************************************
 *   Copyright (c) 2008-2017 Andy Kirkham  All rights reserved.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included
 *   in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 ***********************************************************************************/
 
#ifndef WS_LISTENER_H_INCLUDED
#define WS_LISTENER_H_INCLUDED

struct _listener;
typedef struct _listener   listener_t;
typedef struct _listener * listener_pt;

listener_pt
listener_ctor(const char *inp_ip, short in_port);

void
listener_free(listener_pt inp_self);

void
listener_dtor(listener_pt *inpp_self);

listener_pt
listener_set_flags(listener_pt inp_self, 
	unsigned in_flags);

int
listener_set_ipaddr(listener_pt inp_self, 
	const char *inp_addr, int in_len);

listener_pt
listener_set_backlog(listener_pt inp_self,
	int in_backlog);

listener_pt
listener_set_port(listener_pt inp_self, 
	short in_port);

int
listener_bind(listener_pt inp_self);

int
listener_listen(listener_pt inp_self);

int
listener_get_fd(listener_pt inp_self);

#endif /* WS_LISTENER_H_INCLUDED */

