include(cmake/add_sys_test.cmake)
include(cmake/compile_options.cmake)
include(cmake/configure_python.cmake)
include(cmake/logging_level.cmake)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(lib)
add_subdirectory(pubsub)
add_subdirectory(modules)