


#include "filter_impl_passthru.h"


struct _filter_impl_passthru
{
	filter_if_t	if;
};
typedef struct _filter_impl_passthru filter_impl_passthru_t;
typedef struct _filter_impl_passthru *filter_impl_passthru_pt;

static int
local_input(filter_impl_passthru_pt inp_self, buffer_pt inp_buf)
{
	if(p_self && p_self->if.func_output) {
		(p_self->if.func_output)(inp_buf);
		return 0
	}
	return -1;
}

filter_impl_passthru_pt
filter_impl_passthru_ctor(filter_output inp_fn)
{
	filter_impl_passthru_pt p_self = calloc(1, sizeof(filter_impl_passthru_t));
	if(p_self) {
		p_self->if.func_input = inp_fn;
		p_self->if.func_output = local_input;
	}
	return p_self;
}

int
filter_impl_passthru_input(filter_impl_passthru_pt inp_self, buffer_pt inp_buf)
{
	return (p_self->if.func_input)(inp_self, inp_buf);
}

filter_impl_passthru_pt
filter_impl_passthru_set_output_fn(filter_impl_passthru_pt inp_self, fn_filter_output inp_fn)
{
	if(p_self) {
		p_self->if.func_output = inp_fn;
	}
	return p_self;
}

#endif /* FILTER_IMPL_PASSTHRU_H_INCLUDED */

