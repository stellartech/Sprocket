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

#include "../src/thd_worker.h"

thd_wkr_queue_pt p_queue;

void setup(void)
{
	p_queue = thd_wkr_queue_ctor();
}

void teardown(void)
{
	thd_wkr_queue_dtor(&p_queue);
	ck_assert(p_queue == NULL);
}

START_TEST(test_thd_wkr_queue_ctor)
{
	ck_assert(p_queue != NULL);
}
END_TEST

static void _test_fptr(void *inp) {}

START_TEST(test_thd_wkr_msg_ctor) 
{
	char *p_test = strdup("This quick brown fox");
	thd_wkr_msg_pt p_msg = thd_wkr_msg_ctor(_test_fptr, p_test, free);
	ck_assert(p_msg);
	ck_assert(_test_fptr == thd_wkr_msg_get_func(p_msg));
	ck_assert_str_eq(p_test, thd_wkr_msg_get_args(p_msg));
	thd_wkr_msg_dtor(&p_msg);
	ck_assert(p_msg == NULL);
}
END_TEST

START_TEST(test_thd_wkr_queue)
{
	char *p_test = strdup("This quick brown fox");
        thd_wkr_msg_pt p_rval = NULL, p_msg = thd_wkr_msg_ctor(_test_fptr, p_test, free);
	thd_wkr_queue_push_back(p_queue, p_msg);
	ck_assert(1 == thd_wkr_queue_size(p_queue));
	p_rval = thd_wkr_queue_pop(p_queue);
	ck_assert(0 == thd_wkr_queue_size(p_queue));
	ck_assert(p_rval == p_msg);
	thd_wkr_msg_dtor(&p_rval);
	ck_assert(p_rval == NULL);
}
END_TEST

START_TEST(test_thd_wkr_queue_blocking)
{
	// Single threaded test, should never block, just test functionality
	char *p_test = strdup("This quick brown fox");
        thd_wkr_msg_pt p_rval = NULL, p_msg = thd_wkr_msg_ctor(_test_fptr, p_test, free);
	thd_wkr_queue_push_back_blocking(p_queue, p_msg);
	ck_assert(1 == thd_wkr_queue_size(p_queue));
	p_rval = thd_wkr_queue_pop_blocking(p_queue);
	ck_assert(0 == thd_wkr_queue_size(p_queue));
	ck_assert(p_rval == p_msg);
	thd_wkr_msg_dtor(&p_rval);
	ck_assert(p_rval == NULL);
}
END_TEST


Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("THD_WKR_QUEUE");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_thd_wkr_queue_ctor);
	tcase_add_test(tc_core, test_thd_wkr_msg_ctor);
	tcase_add_test(tc_core, test_thd_wkr_queue);
	tcase_add_test(tc_core, test_thd_wkr_queue_blocking);
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
	srunner_set_log (sr, "thd_wkr_queue.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

