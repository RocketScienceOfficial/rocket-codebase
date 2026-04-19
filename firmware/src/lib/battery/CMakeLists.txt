add_library(battery STATIC
    battery_interp_model.c
)

target_link_libraries(battery PUBLIC
    obc_debug
)