
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
  
SET(FIND_LIBRARY_USE_LIB64_PATHS 1)
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
	/usr/local/lib
	/usr/lib
)
  
SET(sprocket_SRCS
	listener.c
	utils/ws_magic.c
	utils/iovarr.c
	utils/queue.c
	utils/base64.c
	utils/hashmap.c
	utils/str.c
	utils/strhash.c
	utils/inthash.c
	utils/llist.c
	utils/thpool.c
	utils/buffer.c
	ipaddr.c
	tcp_server/tcp_sock_server.c
	tcp_server/server_conn.c
	reactor.c
	ssl.c
	websocket/ws_ht_parser.c
	websocket/ws_frame_buffer.c
	websocket/ws_frame.c
	websocket/ws_frag.c
	websocket/ws_msg.c
	websocket/ws_wire.c
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
  
