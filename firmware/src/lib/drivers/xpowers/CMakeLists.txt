add_library(drivers_xpowers INTERFACE)

target_sources(drivers_xpowers INTERFACE
    XPowersLibInterface.cpp
)

target_compile_options(drivers_xpowers INTERFACE
    -Wno-sign-compare
    -Wno-unused-parameter
)

target_include_directories(drivers_xpowers INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)