add_subdirectory(wmm)

add_library(geo STATIC
    geo_mag.c
    geo.c
    geo_utils.c
)

target_link_libraries(geo PUBLIC
    sys_debug
    maths
    geo_wmm
)

add_sys_test(geo_tests
    SOURCES tests/geo_tests.cpp
    DEPENDS geo
)