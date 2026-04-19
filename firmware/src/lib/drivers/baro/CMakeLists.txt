add_library(drivers_baro STATIC
    ms56xx_driver.c
)

target_link_libraries(drivers_baro PUBLIC
    drivers_common
)