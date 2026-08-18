add_library(app_pubsub INTERFACE)

target_link_libraries(app_pubsub INTERFACE
    maths
    geo
    datalink
    sys_debug
)

add_sys_test(pubsub_tests
    SOURCES
    tests/message_bus_tests.cpp
    DEPENDS
    app_pubsub
)