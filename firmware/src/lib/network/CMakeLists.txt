if(BUILD_NETWORK)
    add_library(network_lib INTERFACE)

    target_sources(network_lib INTERFACE src/TCPSocket.cpp)

    target_include_directories(network_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

    if(WIN32)
        target_link_libraries(network_lib INTERFACE ws2_32)
    endif()
endif()