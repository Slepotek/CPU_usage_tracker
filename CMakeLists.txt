#Set the minimum version of CMake
cmake_minimum_required(VERSION 3.5)

#Defining project
project(CPUsageTracker LANGUAGES C)
set(CMAKE_C_STANDARD 99)

#Include toolchain file
include(${PROJECT_SOURCE_DIR}/toolchain.cmake)

#add subdirectories
add_subdirectory(lib/reader)
#add_subdirectory(lib/analyzer)
#add_subdirectory(lib/printer)
#add_subdirectory(lib/watchdog)
#add_subdirectory(lib/logger)


#add the main executable
add_executable(CPUsageTracker ${PROJECT_SOURCE_DIR}/src/main.c)

#link libraries
target_link_libraries(CPUsageTracker 
    #analyzer_lib
    reader_lib
    #printer_lib
    #watchdog_lib
    #logger_lib
)
