if(OBC_SITL)
    set(RUNNER_PROFILE sitl)
else()
    set(RUNNER_PROFILE flight)
endif()

set(RUNNER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(RUNNER_GEN_PY ${CMAKE_CURRENT_LIST_DIR}/runner_gen.py)
set(RUNNER_PROFILE_FILE ${CMAKE_CURRENT_LIST_DIR}/profiles/${RUNNER_PROFILE}.json)
set(RUNNER_GEN_SOURCE ${RUNNER_OUT_DIR}/ModulesRunner.cpp)
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
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${RUNNER_GEN_PY} --profile ${RUNNER_PROFILE_FILE} --output-source ${RUNNER_GEN_SOURCE} --output-cmake ${RUNNER_GEN_CMAKE}
        RESULT_VARIABLE RUNNER_GEN_RESULT
    )

    if(NOT RUNNER_GEN_RESULT EQUAL 0)
        message(FATAL_ERROR "runner_gen.py failed with exit code ${RUNNER_GEN_RESULT}")
    endif()

    file(TOUCH ${RUNNER_GEN_STAMP})
endif()

add_library(app_modules_runner STATIC
    ${RUNNER_GEN_SOURCE}
)

target_include_directories(app_modules_runner PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

include(${RUNNER_GEN_CMAKE})