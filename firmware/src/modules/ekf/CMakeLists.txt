add_subdirectory(derivation)

add_module(ekf
    SOURCES
    EKFModule.cpp
    ekf.cpp
    madgwick.c
    DEPENDS
    maths
)