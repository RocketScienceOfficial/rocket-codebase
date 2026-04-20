add_module(sim_uart
    SOURCES
    SimUARTModule.cpp
    DEPENDS
    network_lib
    host_sitl
)