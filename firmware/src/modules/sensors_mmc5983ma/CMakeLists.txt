add_module(sensors_mmc5983ma
    SOURCES
    mmc5983ma_Module.cpp
    mmc5983ma_driver.c
    DEPENDS
    drivers_utils
    maths
)