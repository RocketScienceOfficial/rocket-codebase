# ============= CMAKE VERSION =============
cmake_minimum_required(VERSION 3.13)

# ============= BOARD CONFIG =============
include(../../boards/${BOARD_NAME}/board_hw.cmake)

# ============= PICO SDK SETUP =============
set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR}/board)
set(PICO_BOARD pico_obc)
include(pico-sdk/pico_sdk_init.cmake)

# ============= FREERTOS SETUP =============
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/../common/freertos/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# ============= PROJECT SETUP =============
project(firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ============= PICO INIT =============
pico_sdk_init()

# ============= SETUP HAL LIB =============
add_library(platform_hal INTERFACE)

target_include_directories(platform_hal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../include/hal/include
)

# ============= SETUP OSAL LIB =============
add_library(platform_osal INTERFACE)

target_include_directories(platform_osal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../include/osal/include
)

add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../common/freertos
    ${CMAKE_BINARY_DIR}/freertos
)

# ============= INCLUDE APP =============
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_BINARY_DIR}/src
)

# ============= FIRMWARE TARGET =============
add_executable(firmware
    main.cpp
    hal/adc_driver_pico.c
    hal/flash_driver_pico.c
    hal/gpio_driver_pico.c
    hal/i2c_driver_pico.c
    hal/pwm_driver_pico.c
    hal/spi_driver_pico.c
    hal/stdio_driver_pico.c
    hal/time_driver_pico.c
    hal/uart_driver_pico.c
    hal/ws2812b_driver_pico.c
)
target_link_libraries(firmware
    app_main
    platform_hal
    platform_osal
    freertos_osal
    hardware_adc
    hardware_i2c
    hardware_pwm
    hardware_spi
    hardware_uart
    hardware_flash
    hardware_sync
    hardware_pio
    hardware_dma
    pico_stdlib
    pico_flash
)

# ============= PICO SPECIAL HEADERS =============
set(PICO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/hal/generated)

file(MAKE_DIRECTORY ${PICO_GEN_DIR})

pico_generate_pio_header(firmware
    ${CMAKE_CURRENT_LIST_DIR}/hal/ws2812b_driver_pico.pio
    OUTPUT_DIR ${PICO_GEN_DIR}
)

# ============= PICO OUTPUTS =============
pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 0)
pico_add_extra_outputs(firmware)