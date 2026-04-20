add_library(sys_debug STATIC
    sys_assert.c
    sys_benchmark.c
)

target_link_libraries(sys_debug PUBLIC
    platform_hal
)