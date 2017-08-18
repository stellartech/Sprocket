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

#include "../src/utils/buffer.h"

static unsigned char test_string1[] = "This quick brown fox";
static unsigned char test_string2[] = " foo bar baz";
static unsigned char test_string3[] = "This quick brown fox foo bar baz";

buffer_pt p_buf;

void setup(void)
{
	p_buf = buffer_new(test_string1, sizeof(test_string1)-1);
}

void teardown(void)
{
	ck_assert(p_buf);
}

START_TEST(test_buffer_basic)
{
	char p[sizeof(test_string1)];
	ck_assert_int_eq(sizeof(test_string1)-1, buffer_len(p_buf));
	memset(p, 0, sizeof(test_string1));
	memcpy(p, buffer_ptr(p_buf), sizeof(test_string1));
	ck_assert_str_eq(test_string1, p);
}
END_TEST

START_TEST(test_buffer_append)
{
	ck_assert_int_eq(sizeof(test_string1)-1, buffer_len(p_buf));
	buffer_append(p_buf, test_string2, sizeof(test_string2)-1);
	ck_assert_int_eq(sizeof(test_string3)-1, buffer_len(p_buf));
	ck_assert(0 == memcmp(test_string3, buffer_ptr(p_buf), sizeof(test_string3)-1));

	// buffer is type void*. However, internally it hides a '\0' on the end to make
	// sure any str* functions passed the ->p_buf don't wildly run off the end of
	// the buffer. Test this is so with ck_assert_str_eq() which uses strcmp().
	ck_assert_str_eq(test_string3, buffer_ptr(p_buf));
}
END_TEST

START_TEST(test_buffer_byref)
{
	buffer_pt p_local = buffer_copy_byref(p_buf); // Adds a reference to refcount
	ck_assert(p_local);
	ck_assert_msg(memcmp(buffer_ptr(p_local), test_string1, sizeof(test_string1)-1) == 0,
		"failed memcmp at line %d", __LINE__);
	buffer_free(p_local);
	ck_assert_msg(memcmp(buffer_ptr(p_local), test_string1, sizeof(test_string1)-1) == 0,
		"failed memcmp at line %d", __LINE__);

}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("BUFFER");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_buffer_basic);
	tcase_add_test(tc_core, test_buffer_append);
	tcase_add_test(tc_core, test_buffer_byref);
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
	srunner_set_log (sr, "buffer.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

