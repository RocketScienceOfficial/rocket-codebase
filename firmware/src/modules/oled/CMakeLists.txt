set(TARGET_OLED_SRC OLEDModule_NormalMode.cpp)

if(OLED_TEST_MODE)
    set(TARGET_OLED_SRC OLEDModule_TestMode.cpp)
endif()

add_module(oled
    SOURCES
    OLEDModule_Common.cpp
    ${TARGET_OLED_SRC}
    DEPENDS
    drivers_oled
)