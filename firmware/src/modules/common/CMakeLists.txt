add_library(app_modules_common STATIC
    ModuleLogger.cpp
)

target_link_libraries(app_modules_common PUBLIC
    platform_hal
)

include(add_module.cmake)