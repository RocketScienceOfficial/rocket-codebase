add_module(ign
    SOURCES
    IgnitersModule.cpp
)

add_sys_test(ign_tests
    SOURCES
    tests/ign_tests.cpp
    DEPENDS
    app_modules_ign
    host_sitl
)

add_sys_test(ign_malfunction_tests
    SOURCES
    tests/ign_malfunction_tests.cpp
    DEPENDS
    app_modules_ign
    host_sitl
)
