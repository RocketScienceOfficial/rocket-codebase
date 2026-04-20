add_compile_definitions(
    BOARD_FLASH_PAGE_SIZE=256
    BOARD_FLASH_SECTOR_SIZE=4096
    BOARD_FLASH_SIZE=16777216
)

add_library(platform_board INTERFACE)

target_include_directories(platform_board INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

set(HW_INIT_SOURCE ${CMAKE_CURRENT_LIST_DIR}/hw_init.c)