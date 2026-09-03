# Setup u8g2 library
file(GLOB U8G2_SRCS u8g2/csrc/*.c)

add_library(u8g2_lib STATIC
    ${U8G2_SRCS}
)

target_include_directories(u8g2_lib PUBLIC
    u8g2/csrc
)

if(MSVC)
    target_compile_options(u8g2_lib PRIVATE
        /w
        /Os
        /Gy # -ffunction-sections
        /Gw # -fdata-sections
    )
else()
    target_compile_options(u8g2_lib PRIVATE
        -w
        -Os
        -ffunction-sections
        -fdata-sections
    )
endif()

# Setup mode
set(TARGET_OLED_SRC normal/OLEDModule_NormalMode.cpp)

if(OLED_TEST_MODE)
    set(TARGET_OLED_SRC test_mode/OLEDModule_TestMode.cpp)

    message(STATUS "OLED Test Mode: Including SDL2 for simulation")

    find_package(SDL2 REQUIRED)

    target_sources(u8g2_lib PRIVATE
        test_mode/u8x8_d_sdl_128x64.c
        test_mode/u8x8_sdl_key.c
    )

    target_link_libraries(u8g2_lib PUBLIC
        SDL2
    )

    target_include_directories(u8g2_lib PUBLIC
        ${SDL2_INCLUDE_DIRS}
    )
endif()

# Setup OLED module
add_module(oled
    SOURCES
    OLEDModule_Common.cpp
    ${TARGET_OLED_SRC}
    DEPENDS
    u8g2_lib
)