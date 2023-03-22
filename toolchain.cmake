# Set the compiler paths for GCC and Clang (assuming compilation will be run on Linux)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_C_COMPILER "/usr/bin/clang")

# Define the toolchain for GCC
set(CMAKE_C_FLAGS_GCC "-Wall -Wextra -Wpedantic")

# Define the toolchain for Clang
set(CMAKE_C_FLAGS_CLANG "-Weverything -Wno-c++98-compat")

# Define the toolchain selection logic
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCC}") #recursiv variable setting
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_CLANG}")
endif()