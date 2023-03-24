#Define library target for reader
add_library(buffer_lib buffer.c)

#set include directory so that any *.h file will be visible to the compiler 
target_include_directories(buffer_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>) #TODO: Think about adding install case
