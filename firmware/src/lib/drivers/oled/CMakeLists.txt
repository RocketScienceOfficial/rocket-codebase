file(GLOB U8G2_SRCS u8g2/csrc/*.c)

add_library(drivers_oled STATIC
    ${U8G2_SRCS}
)

target_compile_options(drivers_oled PRIVATE
    -ffunction-sections
    -fdata-sections
    -Os
)

target_link_libraries(drivers_oled PUBLIC
    drivers_common
)

target_include_directories(drivers_oled PUBLIC
    u8g2/csrc
)

if(OLED_TEST_MODE)
    message(STATUS "OLED Test Mode: Including SDL2 for simulation")

    find_package(SDL2 REQUIRED)

    target_sources(drivers_oled PRIVATE
        u8x8_d_sdl_128x64.c
        u8x8_sdl_key.c
    )

    target_link_libraries(drivers_oled PUBLIC
        SDL2
    )

    target_include_directories(drivers_oled PUBLIC
        ${SDL2_INCLUDE_DIRS}
    )
endif()