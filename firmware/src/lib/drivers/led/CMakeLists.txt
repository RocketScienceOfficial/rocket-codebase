add_library(drivers_led STATIC
    ws2812b_driver.c
)

target_link_libraries(drivers_led PUBLIC
    drivers_common
)