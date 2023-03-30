#Define library target for reader
add_library(analyzer_lib analyzer.c)

#set include directory so that any *.h file will be visible to the compiler 
target_include_directories(analyzer_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>) #TODO: Think about adding install case

target_link_libraries(analyzer_lib 
    logger_lib
    buffer_lib
)

#TEST implementation
#add executable
add_executable(analyzer_test ${PROJECT_SOURCE_DIR}/lib/analyzer/test_analyzer.c)

#Link the test executable with the program to test
target_link_libraries(analyzer_test 
    analyzer_lib
)
