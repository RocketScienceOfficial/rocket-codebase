# ===== CHECK FOR REBUILD =====
message(STATUS "Runner profile: ${RUNNER_PROFILE}")

set(RUNNER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(RUNNER_GEN_PY ${CMAKE_CURRENT_LIST_DIR}/runner_gen.py)
set(RUNNER_PROFILE_FILE ${CMAKE_CURRENT_LIST_DIR}/profiles/${RUNNER_PROFILE}.json)
set(RUNNER_GEN_SOURCE ${RUNNER_OUT_DIR}/runner.cpp)
set(RUNNER_GEN_CMAKE ${RUNNER_OUT_DIR}/runner.cmake)
set(RUNNER_GEN_STAMP ${RUNNER_OUT_DIR}/runner.stamp)

file(MAKE_DIRECTORY ${RUNNER_OUT_DIR})

set(RUNNER_NEEDS_GEN FALSE)

if(NOT EXISTS ${RUNNER_GEN_SOURCE} OR NOT EXISTS ${RUNNER_GEN_CMAKE} OR NOT EXISTS ${RUNNER_GEN_STAMP})
    set(RUNNER_NEEDS_GEN TRUE)
elseif(${RUNNER_GEN_PY} IS_NEWER_THAN ${RUNNER_GEN_STAMP})
    set(RUNNER_NEEDS_GEN TRUE)
elseif(${RUNNER_PROFILE_FILE} IS_NEWER_THAN ${RUNNER_GEN_STAMP})
    set(RUNNER_NEEDS_GEN TRUE)
endif()

if(RUNNER_NEEDS_GEN)
    message(STATUS "Generating runner code using profile ${RUNNER_PROFILE_FILE}...")
    
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${RUNNER_GEN_PY} --profile ${RUNNER_PROFILE_FILE} --output-source ${RUNNER_GEN_SOURCE} --output-cmake ${RUNNER_GEN_CMAKE}
        RESULT_VARIABLE RUNNER_GEN_RESULT
    )

    if(NOT RUNNER_GEN_RESULT EQUAL 0)
        message(FATAL_ERROR "runner_gen.py failed with exit code ${RUNNER_GEN_RESULT}")
    endif()

    file(TOUCH ${RUNNER_GEN_STAMP})
endif()

# ===== ENTRY POINT SETUP -----=====
add_library(app_main STATIC
    ${RUNNER_GEN_SOURCE}
    ${HW_INIT_SOURCE}
)

target_link_libraries(app_main PUBLIC
    platform_hal
    platform_osal
    platform_board
    app_pubsub
)

include(${RUNNER_GEN_CMAKE})