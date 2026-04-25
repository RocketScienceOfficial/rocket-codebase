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
        $<$<COMPILE_LANGUAGE:CXX>:/w>
        $<$<COMPILE_LANGUAGE:CXX>:/Wno-c++20-extensions>
        $<$<COMPILE_LANGUAGE:CXX>:/Wno-stringop-overflow>
        $<$<COMPILE_LANGUAGE:CXX>:/Wno-double-promotion>
    )
else()
    target_compile_options(RadioLib PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-overloaded-virtual>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-c++20-extensions>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-stringop-overflow>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-double-promotion>
    )
endif()

target_link_libraries(drivers_radio PUBLIC
    drivers_common
    RadioLib
)