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

#include "../src/utils/queue.h"

#define NUM_OF_ELES 4

queue_pt q = NULL;

void setup(void)
{
	queue_item_pt p_item;
	char *s1 = strdup("element1");
	char *s2 = strdup("element2");
	char *s3 = strdup("element3");
	char *s4 = strdup("element4");
		
	q = queue_ctor();

	p_item = queue_item_ctor(1, s1, strlen(s1), free);
	queue_push_back(q, p_item);

	p_item = queue_item_ctor(1, s2, strlen(s2), free);
	queue_push_back(q, p_item);

	p_item = queue_item_ctor(1, s3, strlen(s3), free);
	queue_push_back(q, p_item);

	p_item = queue_item_ctor(1, s4, strlen(s4), free);
	queue_push_back(q, p_item);

}

void teardown(void)
{
	queue_dtor(&q);
	ck_assert(q == NULL);
}

START_TEST(test_queue_pop_front)
{
	int len = 0;
	queue_item_pt p_item = NULL;

	ck_assert_int_eq(NUM_OF_ELES, queue_size(q));
	p_item = queue_pop_front(q);
	ck_assert_str_eq("element1", queue_item_get_payload(p_item, &len));
	ck_assert_int_eq(strlen("element1"), len);
	queue_item_dtor(&p_item);
	ck_assert(p_item == NULL);
	ck_assert_int_eq(NUM_OF_ELES-1, queue_size(q));

	p_item = queue_pop_front(q);
	ck_assert_str_eq("element2", queue_item_get_payload(p_item, &len));
	ck_assert_int_eq(strlen("element2"), len);
	queue_item_dtor(&p_item);
	ck_assert(p_item == NULL);
	ck_assert_int_eq(NUM_OF_ELES-2, queue_size(q));
}
END_TEST


Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("QUEUE");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_queue_pop_front);
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

