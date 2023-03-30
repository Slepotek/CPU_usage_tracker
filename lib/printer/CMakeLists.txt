#Define library target for reader
add_library(printer_lib printer.c)

#set include directory so that any *.h file will be visible to the compiler 
target_include_directories(printer_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>) #TODO: Think about adding install case 

target_link_libraries(printer_lib
    logger_lib
    main_p_lib
)