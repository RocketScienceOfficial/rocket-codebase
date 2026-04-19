add_subdirectory(wmm)

add_library(geo STATIC
    geo_mag.c
    geo.c
    geo_utils.c
)

target_link_libraries(geo PUBLIC
    obc_debug
    maths
    geo_wmm
)

obc_add_test(geo_tests
    SOURCES tests/geo_tests.cpp
    DEPENDS geo
)