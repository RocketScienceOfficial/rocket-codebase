function(add_sys_test test_name)
    if(BUILD_TESTS)
        set(options "")
        set(oneValueArgs "")
        set(multiValueArgs SOURCES DEPENDS)

        cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        if(NOT ARG_SOURCES)
            message(FATAL_ERROR "add_sys_test No SOURCES provided for module '${test_name}'")
        endif()

        message(STATUS "Adding test: ${test_name}")

        add_executable(${test_name} ${ARG_SOURCES})

        target_link_libraries(${test_name} PRIVATE GTest::gtest_main)

        if(ARG_DEPENDS)
            target_link_libraries(${test_name} PRIVATE ${ARG_DEPENDS})
        endif()

        include(GoogleTest)
        gtest_discover_tests(${test_name})
    endif()
endfunction()