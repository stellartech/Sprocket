

#ifndef FILTER_IF_H_INCLUDED
#define FILTER_IF_H_INCLUDED

#include "utils/buffer.h"

typedef int (*fn_filter_in)(buffer_pt);
typedef int (*fn_filter_output)(buffer_pt);

typedef struct
{
	fn_filter_in		func_input;
	fn_filter_output	func_output;	
}
filter_if_t, *filter_if_pt;

#endif /* FILTER_IF_H_INCLUDED */

