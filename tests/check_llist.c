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

#include "../src/llist.h"


llist_pt p = NULL;

void setup(void)
{
	char *s1 = strdup("element1");
	char *s2 = strdup("element2");
	char *s3 = strdup("element3");
	p = llist_ctor(free);
	LLIST_IF(p)->insert(p, "idx1", s1);
	LLIST_IF(p)->insert(p, "idx2", s2);
	LLIST_IF(p)->insert(p, "idx3", s3);
}

void teardown(void)
{
	LLIST_IF(p)->dtor(&p);
}

START_TEST(test_llist_find)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	ck_assert(LLIST_IF(p)->find(p, "foo") == NULL);
	ck_assert_str_eq("element1", LLIST_IF(p)->find(p, "idx1"));
	ck_assert_str_eq("element2", LLIST_IF(p)->find(p, "idx2"));
	ck_assert_str_eq("element3", LLIST_IF(p)->find(p, "idx3"));
}
END_TEST

START_TEST(test_llist_findl)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	ck_assert(LLIST_IF(p)->findl(p, LLIST_CONST_KEY("foo")) == NULL);
	ck_assert_str_eq("element1", LLIST_IF(p)->findl(p, LLIST_CONST_KEY("idx1")));
	ck_assert_str_eq("element2", LLIST_IF(p)->findl(p, LLIST_CONST_KEY("idx2")));
	ck_assert_str_eq("element3", LLIST_IF(p)->findl(p, LLIST_CONST_KEY("idx3")));
}
END_TEST

START_TEST(test_llist_remove__top)
{
	char *removed;
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	removed = LLIST_IF(p)->remove(p, "idx1");
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
	ck_assert_str_eq("element1", removed);
	free(removed);
}
END_TEST

START_TEST(test_llist_remove__mid)
{
	char *removed;
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	removed = LLIST_IF(p)->remove(p, "idx2");
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
	ck_assert_str_eq("element2", removed);
	free(removed);
}
END_TEST

START_TEST(test_llist_remove__bot)
{
	char *removed;
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	removed = LLIST_IF(p)->remove(p, "idx3");
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
	ck_assert_str_eq("element3", removed);
	free(removed);
}
END_TEST

START_TEST(test_llist_removel__top)
{
	char *removed;
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	removed = LLIST_IF(p)->removel(p, LLIST_CONST_KEY("idx1"));
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
	ck_assert_str_eq("element1", removed);
	free(removed);
}
END_TEST

START_TEST(test_llist_removel__mid)
{
	char *removed;
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	removed = LLIST_IF(p)->removel(p, LLIST_CONST_KEY("idx2"));
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
	ck_assert_str_eq("element2", removed);
	free(removed);
}
END_TEST

START_TEST(test_llist_removel__bot)
{
	char *removed;
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	removed = LLIST_IF(p)->removel(p, LLIST_CONST_KEY("idx3"));
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
	ck_assert_str_eq("element3", removed);
	free(removed);
}
END_TEST

START_TEST(test_llist_delete__top)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	LLIST_IF(p)->delete(p, "idx1");
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
}
END_TEST

START_TEST(test_llist_delete__mid)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	LLIST_IF(p)->delete(p, "idx2");
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
}
END_TEST

START_TEST(test_llist_delete__bot)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	LLIST_IF(p)->delete(p, "idx3");
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
}
END_TEST

START_TEST(test_llist_deletel__top)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	LLIST_IF(p)->deletel(p, LLIST_CONST_KEY("idx1"));
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
}
END_TEST

START_TEST(test_llist_deletel__mid)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	LLIST_IF(p)->deletel(p, LLIST_CONST_KEY("idx2"));
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
}
END_TEST

START_TEST(test_llist_deletel__bot)
{
	ck_assert_int_eq(LLIST_IF(p)->count(p), 3);
	LLIST_IF(p)->deletel(p, LLIST_CONST_KEY("idx3"));
	ck_assert_int_eq(LLIST_IF(p)->count(p), 2);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("LLIST");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_llist_find);
	tcase_add_test(tc_core, test_llist_findl);
	tcase_add_test(tc_core, test_llist_remove__top);
	tcase_add_test(tc_core, test_llist_remove__mid);
	tcase_add_test(tc_core, test_llist_remove__bot);
	tcase_add_test(tc_core, test_llist_removel__top);
	tcase_add_test(tc_core, test_llist_removel__mid);
	tcase_add_test(tc_core, test_llist_removel__bot);
	tcase_add_test(tc_core, test_llist_delete__top);
	tcase_add_test(tc_core, test_llist_delete__mid);
	tcase_add_test(tc_core, test_llist_delete__bot);
	tcase_add_test(tc_core, test_llist_deletel__top);
	tcase_add_test(tc_core, test_llist_deletel__mid);
	tcase_add_test(tc_core, test_llist_deletel__bot);
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
	srunner_set_log (sr, "hashmap.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

