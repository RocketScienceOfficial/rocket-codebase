add_module(state_machine
    SOURCES
    StateMachineModule.cpp
    DEPENDS
    maths
    geo
)

add_sys_test(state_machine_tests
    SOURCES
    tests/state_machine_tests.cpp
    DEPENDS
    app_modules_state_machine
    host_sitl
)