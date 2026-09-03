add_subdirectory(xpowers)

add_module(pmu
    SOURCES
    PMUModule.cpp
    DEPENDS
    xpowers
)