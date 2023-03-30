#define library target for logger
add_library(logger_lib logger.c)

#set include directories
target_include_directories(logger_lib INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(logger_lib
    main_p_lib
)