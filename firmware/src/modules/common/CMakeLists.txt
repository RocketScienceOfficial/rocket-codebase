add_library(app_modules_common INTERFACE)

target_link_libraries(app_modules_common INTERFACE
    sys_debug
)

include(add_module.cmake)