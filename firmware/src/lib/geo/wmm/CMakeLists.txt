set(DOWNLOAD_NOAA_MAG_TABLE OFF CACHE BOOL "Whether to download the WMM tables from NOAA. Set to OFF to use a local copy of the tables.")

if(DOWNLOAD_NOAA_MAG_TABLE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/mag_tables.h
        COMMAND Python3::Interpreter ${CMAKE_CURRENT_LIST_DIR}/fetch_noaa_mag_table.py
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/fetch_noaa_mag_table.py
        COMMENT "Generating WMM tables..."
    )
endif()

add_library(geo_wmm INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mag_tables.h
)