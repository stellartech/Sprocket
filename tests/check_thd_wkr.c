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

#include "../src/thd_worker.h"

thd_wkr_queue_pt p_queue_in, p_queue_out;

void setup(void)
{
	p_queue_in  = thd_wkr_queue_ctor();
	p_queue_out = thd_wkr_queue_ctor();
}

void teardown(void)
{
	thd_wkr_queue_dtor(&p_queue_in);
	ck_assert(p_queue_in == NULL);
	thd_wkr_queue_dtor(&p_queue_out);
	ck_assert(p_queue_out == NULL);
}

static void _test_fptr(void *inp) 
{
	// Call creates buffer big enough for this test.
	memset(inp, 0, sizeof("0123456789\0")-1);
	memcpy(inp, "0123456789\0", sizeof("0123456789\0")-1);
}

START_TEST(test_thd_wkr) 
{
	char *p_testbuf;
	thd_wkr_msg_pt p_msg;
	thd_wkr_pt p_thread = thd_wkr_ctor(p_queue_in, p_queue_out);
	ck_assert(p_thread != NULL);
	p_testbuf = calloc(1, 32);
	// Note, we don't free the msg args as we need
	// the copy from _test_fptr() to test against.  vvvv
	p_msg = thd_wkr_msg_ctor(_test_fptr, p_testbuf, NULL); 
	thd_wkr_queue_push_back(p_queue_in, p_msg);
	usleep(1000); // Allow thread function to invoke.
	thd_wkr_dtor(&p_thread); // Blocks until thread end.
	ck_assert(p_thread == NULL);
	ck_assert_str_eq(p_testbuf, "0123456789"); 
	free(p_testbuf);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("THD_WKR_QUEUE");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_thd_wkr);
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
	srunner_set_log (sr, "thd_wkr.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

