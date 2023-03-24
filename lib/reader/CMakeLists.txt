#Define library target for reader
add_library(reader_lib reader.c)

#set include directory so that any *.h file will be visible to the compiler 
target_include_directories(reader_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>) #TODO: Think about adding install case

#TEST implementation
#add executable
add_executable(reader_test ${PROJECT_SOURCE_DIR}/lib/reader/test_reader.c)

#Link the test executable with the program to test
target_link_libraries(reader_test reader_lib)
#move neccessary test_text to build directory
add_custom_command(
    TARGET reader_test
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/test_text"
        "$<TARGET_FILE_DIR:reader_test>/test_text"
)
