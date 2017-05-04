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

#include "../src/thpool.h"

// Simple test based on:
// https://github.com/Pithikos/C-Thread-Pool/blob/master/tests/src/api.c

typedef struct
{
	int sleep_for;
} 
thd_func_args_t;

void sleep_secs(void* inp)
{
	thd_func_args_t *p_args = (thd_func_args_t*)inp;
	sleep(p_args->sleep_for);
}

threadpool thpool;

void setup(void)
{
	thpool = thpool_init(10);
}

void teardown(void)
{
	thpool_destroy(thpool);
}

START_TEST(test_th_pool) 
{
	int num = 0;
	thd_func_args_t args = { .sleep_for = 1 }; // RO so should be ok to share
	thpool_add_work(thpool, (void*)sleep_secs, &args);
	thpool_add_work(thpool, (void*)sleep_secs, &args);
	thpool_add_work(thpool, (void*)sleep_secs, &args);
	thpool_add_work(thpool, (void*)sleep_secs, &args);
	usleep(100);
	num = thpool_num_threads_working(thpool);
	ck_assert_int_eq(4, thpool_num_threads_working(thpool));
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("THD_WKR_POOL");

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
	srunner_set_log (sr, "thd_pool.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

