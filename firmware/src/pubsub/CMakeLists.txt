add_library(app_pubsub STATIC
    MessageBus.cpp
)

target_link_libraries(app_pubsub PUBLIC
    maths
    geo
    datalink
    sys_debug
)