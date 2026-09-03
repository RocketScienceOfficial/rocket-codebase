add_library(drivers_utils STATIC
    bus_utils.c
    spi_utils.c
    time_utils.c
)

target_link_libraries(drivers_utils PUBLIC
    platform_hal
    platform_osal
    sys_debug
)

target_include_directories(drivers_utils PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)