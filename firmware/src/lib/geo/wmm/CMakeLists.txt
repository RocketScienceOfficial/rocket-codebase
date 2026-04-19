add_custom_command(
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/mag_tables.h
    COMMAND Python3::Interpreter ${CMAKE_CURRENT_LIST_DIR}/fetch_noaa_mag_table.py
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/fetch_noaa_mag_table.py
    COMMENT "Generating WMM tables..."
)

add_library(geo_wmm INTERFACE ${CMAKE_CURRENT_LIST_DIR}/mag_tables.h)