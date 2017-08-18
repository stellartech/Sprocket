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

#include "../src/utils/str.h"

#define STR1 "This quick brown fox"
#define STR2 " jumped over the cow"

str_pt p_str;

void setup(void)
{
	p_str = str_ctor(STR1, strlen(STR1));
}

void teardown(void)
{
	str_decref(p_str);
}

START_TEST(test_str_ctor)
{
	ck_assert(p_str);
}
END_TEST

START_TEST(test_str_steal_ctor)
{
	int len = -1;
	str_pt p_temp = str_steal_ctor(
		strndup(STR1, strlen(STR1)), // Returned pointer is "owned" by p_temp
		strlen(STR1));               // and is free()ed by str_decref() below
	ck_assert_str_eq(STR1, str_get_with_len(p_temp, &len));
	ck_assert_int_eq(strlen(STR1), len);
	str_decref(p_temp);
}
END_TEST

START_TEST(test_str_get_len)
{
	ck_assert(p_str);
	ck_assert_int_eq(strlen(STR1), str_get_len(p_str));
}
END_TEST

START_TEST(test_str_copy_byref)
{
	const char *s;
	str_pt p_str2;
	ck_assert(p_str);
	ck_assert_int_eq(1, str_get_refcount(p_str));
	p_str2 = str_copy_byref(p_str);
	ck_assert_int_eq(2, str_get_refcount(p_str2));
	s = str_get(p_str);
	ck_assert_str_eq(STR1, s);
	s = str_get(p_str2);
	ck_assert_str_eq(STR1, s);
	str_decref(p_str2);
	ck_assert_int_eq(1, str_get_refcount(p_str));
}
END_TEST

START_TEST(test_str_copy_ctor)
{
	const char *s;
	str_pt p_str2;
	ck_assert(p_str);
	p_str2 = str_copy_ctor(p_str);
	s = str_get(p_str);
	ck_assert_str_eq(STR1, s);
	s = str_get(p_str2);
	ck_assert_str_eq(STR1, s);
	str_decref(p_str2);
	ck_assert_int_eq(1, str_get_refcount(p_str));
}
END_TEST

START_TEST(test_str_concat)
{
	const char *s;
	str_pt p_str2 = str_ctor(STR2, strlen(STR2));
	ck_assert(p_str);
	s = str_get(p_str);
	ck_assert_str_eq(STR1, s);
	ck_assert(p_str2);
	s = str_get(p_str2);
	ck_assert_str_eq(STR2, s);

	str_concat(p_str, p_str2);
	ck_assert_int_eq(strlen(STR1) + strlen(STR2), str_get_len(p_str));
	s = str_get(p_str);
	ck_assert_str_eq(STR1 STR2, s);
	str_decref(p_str2);
}
END_TEST

START_TEST(test_str_get_with_len)
{
	int len = 0;
	str_get_with_len(p_str, &len);
	ck_assert_int_eq(strlen(STR1), len);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("STR");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_str_ctor);
	tcase_add_test(tc_core, test_str_steal_ctor);
	tcase_add_test(tc_core, test_str_copy_byref);
	tcase_add_test(tc_core, test_str_copy_ctor);
	tcase_add_test(tc_core, test_str_get_len);
	tcase_add_test(tc_core, test_str_concat);
	tcase_add_test(tc_core, test_str_get_with_len);
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
	srunner_set_log (sr, "str.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

