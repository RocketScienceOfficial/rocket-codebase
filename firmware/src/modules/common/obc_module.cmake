function(obc_add_module module_name)
    set(lib_name app_modules_${module_name})
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES DEPENDS)

    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "obc_add_module: No SOURCES provided for module '${module_name}'")
    endif()

    add_library(${lib_name} STATIC ${ARG_SOURCES})

    target_link_libraries(${lib_name} PUBLIC
        platform_hal
        platform_osal
        platform_board
        app_pubsub
        app_modules_common
    )

    if(ARG_DEPENDS)
        target_link_libraries(${lib_name} PUBLIC ${ARG_DEPENDS})
    endif()

    target_compile_definitions(${lib_name} PRIVATE
        MODULE_NAME="${module_name}"
    )
endfunction()