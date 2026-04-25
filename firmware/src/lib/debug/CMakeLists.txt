add_library(sys_debug STATIC
    sys_assert.c
    sys_benchmark.c
)

target_link_libraries(sys_debug PUBLIC
    platform_hal
)

set(DEBUG_ASSERTS_ENABLED OFF)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(DEBUG_ASSERTS_ENABLED ON)
endif()

message(STATUS "Enabling Asserts: ${DEBUG_ASSERTS_ENABLED}")