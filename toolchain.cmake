# Define the toolchain for GCC
set(CMAKE_C_FLAGS_GCC "-Wall -Wextra -Wpedantic")

# Define the toolchain for Clang
set(CMAKE_C_FLAGS_CLANG "-Weverything -Wno-c++98-compat")

# Define the toolchain selection logic
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCC}") #recursiv variable setting
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_CLANG}")
endif()