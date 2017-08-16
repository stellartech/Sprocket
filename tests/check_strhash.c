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
#include <stdlib.h>

#include "../src/strhash.h"

#define INDEX_STR "index_str"
#define TEST_STR "Test Me"

strhash_pt p = NULL;
str_pt p_key = NULL;

void setup(void)
{
	p_key = str_ctor(INDEX_STR, strlen(INDEX_STR));
	char *p_value = strdup(TEST_STR);
	p = strhash_ctor(32, free);
	strhash_insert(p, p_key, p_value);
	str_decref(p_key);
}

void teardown(void)
{
	strhash_dtor(&p);
	//str_decref(p_key);
}

START_TEST(test_strhash_find)
{
	char *actual = NULL;
	ck_assert(strhash_count(p) == 1);
	actual = strhash_find(p, p_key);
	ck_assert(actual);
	ck_assert_str_eq(TEST_STR, actual);
}
END_TEST

START_TEST(test_strhash_find_ex)
{
	char *actual = NULL;
	ck_assert(strhash_count(p) == 1);
	actual = strhash_find_ex(p, INDEX_STR);
	ck_assert(actual);
	ck_assert_str_eq(TEST_STR, actual);
}
END_TEST

START_TEST(test_strhash_delete)
{
	char *actual = NULL;
	ck_assert(strhash_count(p) == 1);
	strhash_delete(p, p_key);
	ck_assert(strhash_count(p) == 0);
}
END_TEST

START_TEST(test_strhash_remove)
{
	char *actual = NULL;
	ck_assert(strhash_count(p) == 1);
	actual = strhash_remove(p, p_key);
	ck_assert(strhash_count(p) == 0);
	ck_assert_str_eq(TEST_STR, actual);
	free(actual);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("STRHASH");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_strhash_find);
	tcase_add_test(tc_core, test_strhash_find_ex);
	tcase_add_test(tc_core, test_strhash_delete);
	tcase_add_test(tc_core, test_strhash_remove);
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
	srunner_set_log (sr, "strhash.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

