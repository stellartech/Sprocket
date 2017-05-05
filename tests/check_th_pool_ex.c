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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>

#include "../src/thpool.h"

#define SOCKET_ADDRESS "inproc://a"

int pull_sock;
threadpool thpool;

typedef struct 
{
	int	nano_push_sock;
}
thd_resource_t;

void*
thd_resource_init(void *inp) 
{
	thd_resource_t* p_resource = calloc(1, sizeof(thd_resource_t));
	if(p_resource) {
		p_resource->nano_push_sock = nn_socket(AF_SP, NN_PUSH);
		nn_connect(p_resource->nano_push_sock, SOCKET_ADDRESS);
	}
	return p_resource;	
}

void
thd_resource_free(void* inp)
{
	thd_resource_t* p_resource = (thd_resource_t*)inp;
	nn_close(p_resource->nano_push_sock);
	free(inp);
}

void setup(void)
{
	pull_sock = nn_socket(AF_SP, NN_PULL);
	nn_bind(pull_sock, SOCKET_ADDRESS);
	thpool = thpool_init_ex(10, thd_resource_init, NULL, thd_resource_free);
}

void teardown(void)
{
	thpool_destroy(thpool);
	nn_close(pull_sock);
}

typedef struct
{
	char *p;
} 
thd_func_args_t;

void thdfunc(void* inp, void* inp_resources)
{
	thd_func_args_t *p_args = (thd_func_args_t*)inp;
	thd_resource_t  *p_resources = (thd_resource_t*)inp_resources;
	if(p_resources && p_resources->nano_push_sock) {
		nn_send(p_resources->nano_push_sock, p_args->p, 4, NN_DONTWAIT);
	}
}

static int check_rval(char *inp) 
{
	// msgs can arrive out of order
	if(strcmp(inp, "ABC") == 0) return 1;
	if(strcmp(inp, "DEF") == 0) return 1;
	if(strcmp(inp, "GHI") == 0) return 1;
	if(strcmp(inp, "JKL") == 0) return 1;
	return 0;
}

START_TEST(test_th_pool) 
{
	thd_func_args_t args1 = { .p = "ABC" }; 
	thd_func_args_t args2 = { .p = "DEF" }; 
	thd_func_args_t args3 = { .p = "GHI" }; 
	thd_func_args_t args4 = { .p = "JKL" }; 
	thpool_add_work_ex(thpool, (void*)thdfunc, &args1);
	thpool_add_work_ex(thpool, (void*)thdfunc, &args2);
	thpool_add_work_ex(thpool, (void*)thdfunc, &args3);
	thpool_add_work_ex(thpool, (void*)thdfunc, &args4);
	for(int i = 0; i < 4; i++) {
		char rx[32];
		nn_recv(pull_sock, rx, sizeof(rx), 0);
		ck_assert(check_rval(rx));
	}
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("TH_POOL_EX");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_set_timeout(tc_core, 10.0);
	tcase_add_test(tc_core, test_th_pool);
	suite_add_tcase(s, tc_core);

	return s;
}

int 
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	s = suite();
	sr = srunner_create(s);
	srunner_set_log (sr, "thd_pool_ex.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

