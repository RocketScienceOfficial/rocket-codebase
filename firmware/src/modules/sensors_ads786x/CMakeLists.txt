add_library(drivers_adc STATIC
    ads786x_driver.c
)

target_link_libraries(drivers_adc PUBLIC
    drivers_common
)