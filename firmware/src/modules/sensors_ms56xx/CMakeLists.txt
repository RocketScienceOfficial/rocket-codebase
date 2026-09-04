add_module(sensors_ms56xx
    SOURCES
    ms56xx_Module.cpp
    ms56xx_driver.c
    DEPENDS
    drivers_utils
    geo
)