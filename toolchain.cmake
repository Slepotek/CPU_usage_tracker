# Define the toolchain for GCC
set(CMAKE_C_FLAGS_GCC "-Wall -Wextra")# -Wno-unused-function -Wno-incompatible-pointer-types")

# Define the toolchain for Clang
set(CMAKE_C_FLAGS_CLANG "-Weverything")# -Wno-c++98-compat -Wno-declaration-after-statement -Wno-unused-function -Wno-incompatible-function-pointer-types -Wno-pedantic -Wno-vla -Wno-disabled-macro-expansion")

# Define the toolchain selection logic
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCC}") #recursiv variable setting
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_CLANG}")
endif()