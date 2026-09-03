add_library(drivers_mag STATIC
    mmc5983ma_driver.c
)

target_link_libraries(drivers_mag PUBLIC
    drivers_common
    maths
)