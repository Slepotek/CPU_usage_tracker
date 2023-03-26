#Set the minimum version of CMake
cmake_minimum_required(VERSION 3.5)

#Defining project
project(CPUsageTracker LANGUAGES C)
set(CMAKE_C_STANDARD 99)



#Set the C compiler for the environment variable CC
if(DEFINED ENV{CC})
    #acquire the compiler from CC variable
    set(CMAKE_C_COMPILER $ENV{CC})
endif()

message("The compiler is ${CMAKE_C_COMPILER_ID}")

#Include toolchain file with compiler specific flags
include(${PROJECT_SOURCE_DIR}/toolchain.cmake)

#add subdirectories
add_subdirectory(lib/reader)
add_subdirectory(lib/buffer)
add_subdirectory(lib/analyzer)
#add_subdirectory(lib/printer)
#add_subdirectory(lib/watchdog)
#add_subdirectory(lib/logger)

#Define library target for reader
add_library(main_p_lib src/main_p.c)

#set include directory so that any *.h file will be visible to the compiler 
target_include_directories(main_p_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>, lib/reader, lib/analyzer) #TODO: Think about adding install case

#add the main executable
add_executable(CPUsageTracker ${PROJECT_SOURCE_DIR}/src/main.c)

#link libraries
target_link_libraries(CPUsageTracker 
    analyzer_lib
    reader_lib
    buffer_lib
    main_p_lib
    #printer_lib
    #watchdog_lib
    #logger_lib
)

target_link_libraries(main_p_lib 
    analyzer_lib
    reader_lib
)

#TESTS
enable_testing()

#reader TEST
add_test(NAME ReaderTest COMMAND $<TARGET_FILE:reader_test>)
#buffer TEST
add_test(NAME BufferTest COMMAND $<TARGET_FILE:buffer_test>)
#analyzer TEST
add_test(NAME AnalyzerTest COMMAND $<TARGET_FILE:analyzer_test>)
