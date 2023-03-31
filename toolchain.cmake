# Define the toolchain for GCC
set(CMAKE_C_FLAGS_GCC "-Wall -Wextra -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-format-truncation")# -Wno-unused-function -Wno-incompatible-pointer-types")

# Define the toolchain for Clang
set(CMAKE_C_FLAGS_CLANG "-Weverything -Wno-unused-parameter -Wno-disabled-macro-expansion -Wno-declaration-after-statement -Wno-incompatible-function-pointer-types -Wno-pedantic -Wno-vla")

# Define the toolchain selection logic
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_GCC}") #recursiv variable setting
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_CLANG}")
endif()