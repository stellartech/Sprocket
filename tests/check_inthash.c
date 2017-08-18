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

#include "../src/utils/inthash.h"

#define STR0 "1Test Me1"
#define STR1 "2Test Me2"
#define STR33 "3Test Me3"

inthash_pt p = NULL;

void setup(void)
{
	p = inthash_ctor(32, free);
	inthash_insert(p,  0, strdup(STR0));
	inthash_insert(p,  1, strdup(STR1));
	inthash_insert(p, 33, strdup(STR33));
}

void teardown(void)
{
	inthash_dtor(&p);
}

START_TEST(test_inthash_find)
{
	char *actual = NULL;
	ck_assert(inthash_count(p) == 3);
	actual = inthash_find(p, 0);
	ck_assert(actual);
	ck_assert_str_eq(STR0, actual);
	actual = inthash_find(p, 1);
	ck_assert(actual);
	ck_assert_str_eq(STR1, actual);
	actual = inthash_find(p, 33);
	ck_assert(actual);
	ck_assert_str_eq(STR33, actual);
}
END_TEST

START_TEST(test_inthash_delete)
{
	char *actual = NULL;
	ck_assert(inthash_count(p) == 3);
	inthash_delete(p, 0);
	ck_assert(inthash_count(p) == 2);
}
END_TEST

START_TEST(test_inthash_remove)
{
	char *actual = NULL;
	ck_assert(inthash_count(p) == 3);
	actual = inthash_remove(p, 0);
	ck_assert(inthash_count(p) == 2);
	ck_assert_str_eq(STR0, actual);
	free(actual);
	actual = inthash_remove(p, 1);
	ck_assert(inthash_count(p) == 1);
	ck_assert_str_eq(STR1, actual);
	free(actual);
	actual = inthash_remove(p, 33);
	ck_assert(inthash_count(p) == 0);
	ck_assert_str_eq(STR33, actual);
	free(actual);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("INTHASH");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_inthash_find);
	tcase_add_test(tc_core, test_inthash_delete);
	tcase_add_test(tc_core, test_inthash_remove);
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
	srunner_set_log (sr, "inthash.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

