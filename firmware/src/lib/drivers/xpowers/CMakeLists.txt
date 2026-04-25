add_library(drivers_xpowers INTERFACE)

target_sources(drivers_xpowers INTERFACE
    XPowersLibInterface.cpp
)

target_include_directories(drivers_xpowers INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)