add_library(gps STATIC
    gps_driver.c
    nmea.c
    ubx.c
)

target_link_libraries(gps PUBLIC
    geo
)

add_sys_test(gps_tests
    SOURCES
    tests/nmea_tests.cpp
    tests/ubx_tests.cpp
    DEPENDS
    gps
)