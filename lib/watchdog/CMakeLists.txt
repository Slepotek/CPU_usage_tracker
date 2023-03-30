#Define library target for reader
add_library(watchdog_lib watchdog.c)

#set include directory so that any *.h file will be visible to the compiler 
target_include_directories(watchdog_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>) #TODO: Think about adding install case

target_link_libraries(watchdog_lib 
    logger_lib
    printer_lib
    reader_lib
    analyzer_lib
)