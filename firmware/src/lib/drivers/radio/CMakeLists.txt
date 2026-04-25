set(ESP_PLATFORM OFF CACHE BOOL "" FORCE)

add_subdirectory(RadioLib)

unset(ESP_PLATFORM CACHE)

target_compile_options(RadioLib PRIVATE
    -Wno-stringop-overflow
    -Wno-overloaded-virtual
    -Wno-double-promotion
)

target_compile_definitions(RadioLib PRIVATE
    RADIOLIB_STATIC_ONLY
)

add_library(drivers_radio STATIC
    RadioLibHALPort.cpp
)

target_compile_options(drivers_radio PRIVATE
    -Wno-overloaded-virtual
)

target_link_libraries(drivers_radio PUBLIC
    drivers_common
    RadioLib
)