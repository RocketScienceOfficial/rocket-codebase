# HAL required variables
add_compile_definitions(
    BOARD_FLASH_PAGE_SIZE=256
    BOARD_FLASH_SECTOR_SIZE=4096
    BOARD_FLASH_SIZE=16777216
)

# Runner required library
add_library(hw_info INTERFACE)

target_include_directories(hw_info INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(hw_info INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/hw_init.c
)