add_library(app_pubsub STATIC
    MessageBus.cpp
)

target_link_libraries(app_pubsub PUBLIC
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