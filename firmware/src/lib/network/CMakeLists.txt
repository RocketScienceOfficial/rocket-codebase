if(BUILD_NETWORK)
    message(STATUS "Building network library")
    
    add_library(network_lib STATIC
        src/TCPSocket.cpp
    )

    target_include_directories(network_lib PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
    )

    target_link_libraries(network_lib PUBLIC
        datalink
        sys_debug
    )

    if(WIN32)
        target_link_libraries(network_lib PRIVATE
            ws2_32
        )
    endif()
endif()