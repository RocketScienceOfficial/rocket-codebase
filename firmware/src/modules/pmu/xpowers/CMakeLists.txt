add_library(xpowers INTERFACE)

target_sources(xpowers INTERFACE
    XPowersLibInterface.cpp
)

target_include_directories(xpowers INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)