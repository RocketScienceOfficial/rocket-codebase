# ============= SETUP =============
cmake_minimum_required(VERSION 3.31)

project(firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ============= BOARD CONFIG =============
include(../../boards/${BOARD_NAME}/board_hw.cmake)

# ============= TESTS SETUP =============
if(BUILD_TESTS)
    include(CTest)
    include(FetchContent)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # MSVC runtime compatibility for gtest

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
    )
    FetchContent_MakeAvailable(googletest)
endif()

# ============= SETUP HAL LIB =============
add_library(platform_hal STATIC
    hal/adc_driver_host.c
    hal/flash_driver_host.c
    hal/gpio_driver_host.c
    hal/i2c_driver_host.c
    hal/pwm_driver_host.c
    hal/spi_driver_host.c
    hal/stdio_driver_host.c
    hal/uart_driver_host.c
    hal/ws2812b_driver_host.c
)
target_include_directories(platform_hal PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include/hal/include
)

# ============= SETUP OSAL LIB =============
find_package(Threads REQUIRED)

if(SITL_FREERUN)
    set(SITL_SOURCE sitl/sitl_freerun.cpp)
else()
    set(SITL_SOURCE sitl/sitl_lockstep.cpp)
endif()

add_library(platform_osal STATIC
    ${SITL_SOURCE}
)
target_link_libraries(platform_osal PRIVATE
    platform_hal
    Threads::Threads
)
target_include_directories(platform_osal PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include/osal/include
)

# ============= SETUP SITL LIB =============
add_library(host_sitl INTERFACE)

target_link_libraries(host_sitl INTERFACE
    platform_osal
)
target_include_directories(host_sitl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/sitl
)

# ============= ENABLE NETWORK LIB =============
set(BUILD_NETWORK ON)

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)

if(BUILD_TYPE_UPPER STREQUAL "DEBUG")
    add_compile_definitions(NETWORK_LOGS_ENABLED)
endif()

# ============= INCLUDE APP =============
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_BINARY_DIR}/src
)

# ============= FIRMWARE TARGET =============
add_executable(firmware
    main.cpp
)
target_link_libraries(firmware
    app_main
)