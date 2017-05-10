


#ifndef WS_FRAME_BUFFER_H_INCLUDED
#define WS_FRAME_BUFFER_H_INCLUDED

#include <stdint.h>
#include <event2/buffer.h>

#ifdef __cplusplus
extern C {
#endif

typedef union
{
        unsigned char bytes[2];
        uint16_t len;
} 
ws_frame_len_126_t;

typedef struct
{
        ws_frame_len_126_t len;
        unsigned char mask[4];
} 
ws_frame_len_126_masked_t;

typedef union
{
        unsigned char bytes[8];
        uint64_t len;
} 
ws_frame_len_127_t;

typedef struct
{
        ws_frame_len_127_t len;
        unsigned char mask[4];
} 
ws_frame_len_127_masked_t;

typedef struct
{
	unsigned char control;
	unsigned char short_len;
	union {
		ws_frame_len_126_t len126;
		ws_frame_len_127_t len127;
		ws_frame_len_126_masked_t m_len126;
		ws_frame_len_127_masked_t m_len127;
	} extended;
}
ws_frame_preamble_t;

typedef struct _ws_frame_buffer
{
	int complete : 1;
	int type : 3;
	int mask : 3;
	int offset : 4;
	ws_frame_preamble_t preamble;	
	struct evbuffer *p_evbuffer;
	struct _ws_frame_buffer *p_next;
} 
ws_frame_buffer_t, *ws_frame_buffer_pt;


// Constructor
ws_frame_buffer_pt
ws_frame_buffer_ctor(void);

// Free function
void
ws_frame_buffer_free(void*);

// Destructor
void
ws_frame_buffer_dtor(ws_frame_buffer_pt *inpp_self);

int64_t
ws_frame_append(ws_frame_buffer_pt p_self, struct evbuffer *inp_evbuufer);

#ifdef __cplusplus
}
#endif



#endif /* WS_FRAME_BUFFER_H_INCLUDED */

