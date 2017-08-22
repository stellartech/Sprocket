

#ifndef FILTER_IMPL_PASSTHRU_H_INCLUDED
#define FILTER_IMPL_PASSTHRU_H_INCLUDED

#include "filter_if.h"


struct _filter_impl_passthru
{
	filter_if_t	if;
};
typedef struct _filter_impl_passthru filter_impl_passthru_t;
typedef struct _filter_impl_passthru *filter_impl_passthru_pt;


filter_impl_passthru_pt
filter_impl_passthru_ctor(filter_output);

int
filter_impl_passthru_input(filter_impl_passthru_pt inp_self, buffer_pt inp_buf);

filter_impl_passthru_pt
filter_impl_passthru_set_output_fn(filter_impl_passthru_pt inp_self, fn_filter_output inp_fn);

#endif /* FILTER_IMPL_PASSTHRU_H_INCLUDED */

