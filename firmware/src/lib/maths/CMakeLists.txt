add_library(maths STATIC
    fast_math.c
    math_utils.c
    quaternion.c
    vector.c
)

target_link_libraries(maths PUBLIC
    sys_debug
)

target_compile_options(maths PRIVATE
    -Wno-strict-aliasing
    -Wno-uninitialized
)

add_sys_test(math_tests
    SOURCES tests/maths_tests.cpp
    DEPENDS maths
)