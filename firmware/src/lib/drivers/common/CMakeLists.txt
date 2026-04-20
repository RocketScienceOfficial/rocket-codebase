add_library(drivers_common STATIC
    bus_utils.c
    spi_utils.c
    time_utils.c
)

target_link_libraries(drivers_common PUBLIC
    platform_hal
    platform_osal
    sys_debug
)

target_include_directories(drivers_common PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)