add_subdirectory(RadioLib)

target_compile_options(RadioLib PRIVATE
    -Wno-stringop-overflow
    -Wno-overloaded-virtual
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