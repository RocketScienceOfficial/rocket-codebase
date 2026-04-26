set(ESP_PLATFORM OFF CACHE BOOL "" FORCE)

add_subdirectory(RadioLib SYSTEM)

unset(ESP_PLATFORM CACHE)

target_compile_definitions(RadioLib PRIVATE
    RADIOLIB_STATIC_ONLY
)

add_library(drivers_radio STATIC
    RadioLibHALPort.cpp
)

if(MSVC)
    target_compile_options(RadioLib PRIVATE
       /w
       /Wno-c++20-extensions
       /Wno-stringop-overflow
       /Wno-double-promotion
    )
else()
    target_compile_options(RadioLib PRIVATE
        -Wno-overloaded-virtual
        -Wno-c++20-extensions
        -Wno-stringop-overflow
        -Wno-double-promotion
    )
endif()

target_link_libraries(drivers_radio PUBLIC
    drivers_common
    RadioLib
)