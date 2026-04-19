add_library(obc_debug STATIC
    obc_assert.c
    obc_benchmark.c
)

target_link_libraries(obc_debug PUBLIC
    platform_hal
)