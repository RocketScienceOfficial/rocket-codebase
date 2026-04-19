add_library(maths STATIC
    fast_math.c
    math_utils.c
    quaternion.c
    vector.c
)

target_link_libraries(maths PUBLIC
    obc_debug
)

obc_add_test(math_tests
    SOURCES tests/maths_tests.cpp
    DEPENDS maths
)