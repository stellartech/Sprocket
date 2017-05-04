
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
  
SET(FIND_LIBRARY_USE_LIB64_PATHS 1)
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
	/usr/local/lib
	/usr/lib
)
  
SET(sprocket_SRCS
	ws_ht_parser.c
	ws_magic.c
	hashmap.c
	base64.c
	llist.c
	queue.c
	#sha1.c
	#tcp_listener.c
	thd_worker.c
)
  
ADD_LIBRARY(sprocket
    ${sprocket_SRCS}
)
  
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

TARGET_LINK_LIBRARIES(sprocket
	jansson
	http_parser
	ssl
	crypto
	event
	event_extra
	event_openssl
	event_pthreads
)
  
