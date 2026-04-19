if(NOT DEFINED OBC_LOG_LEVEL)
    message(FATAL_ERROR "OBC_LOG_LEVEL not set.")
endif()

add_compile_definitions(OBC_LOG_LEVEL=${OBC_LOG_LEVEL})