add_subdirectory(wmm)

add_library(geo STATIC
    geo_mag.c
    geo_utils.c
    projection.c
    wgs84.c
)

target_link_libraries(geo PUBLIC
    geo_wmm
    maths
    sys_debug
)

add_sys_test(geo_tests
    SOURCES
    tests/geo_tests.cpp
    DEPENDS
    geo
    maths
)