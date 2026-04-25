# ============= CMAKE VERSION =============
cmake_minimum_required(VERSION 3.16)

# ============= BOARD CONFIG =============
include(../../boards/${BOARD_NAME}/board_hw.cmake)

# ============= ESP SDK SETUP =============
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ============= PROJECT SETUP =============
project(firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

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

# ============= INCLUDE APP =============
add_subdirectory(
    ${CMAKE_CURRENT_LIST_DIR}/../../src
    ${CMAKE_BINARY_DIR}/src
)