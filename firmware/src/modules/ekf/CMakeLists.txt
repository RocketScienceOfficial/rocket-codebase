add_subdirectory(derivation)

obc_add_module(ekf
    SOURCES
    EKFModule.cpp
    ekf.cpp
    madgwick.c
    DEPENDS
    maths
)