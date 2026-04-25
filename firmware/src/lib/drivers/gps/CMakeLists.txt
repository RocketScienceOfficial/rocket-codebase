add_library(drivers_gps STATIC
    gps_driver.c
    nmea.c
    ubx.c
)

target_link_libraries(drivers_gps PUBLIC
    drivers_common
    geo
)