
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
  
SET(FIND_LIBRARY_USE_LIB64_PATHS 1)
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
	/usr/local/lib
	/usr/lib
)
  
SET(sprocket_SRCS
	ws_ht_parser.c
	ws_magic.c
	ws_frame_buffer.c
	ws_frame.c
	ws_frag.c
	ws_msg.c
	listener.c
	hashmap.c
	base64.c
	llist.c
	buffer.c
	bufvec.c
	queue.c
	thpool.c
	ws_wire.c
	ipaddr.c
	tcp_server/tcp_sock_server.c
	tcp_server/server_conn.c
	reactor.c
	ssl.c
)
  
ADD_LIBRARY(sprocket
    ${sprocket_SRCS}
)
  
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

TARGET_LINK_LIBRARIES(sprocket
	ssl
	config
	crypto
	jansson
	nanomsg
	pthread
	http_parser
	event
	event_extra
	event_openssl
	event_pthreads
)
  
