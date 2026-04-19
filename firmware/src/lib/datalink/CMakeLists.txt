set(DATALINK_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../datalink)
set(DATALINK_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${DATALINK_OUT_DIR})
file(GLOB_RECURSE DATALINK_GEN_SOURCES CONFIGURE_DEPENDS ${DATALINK_DIR}/*)

add_custom_command(
    OUTPUT ${DATALINK_OUT_DIR}/datalink.h ${DATALINK_OUT_DIR}/datalink.c
    COMMAND Python3::Interpreter ${DATALINK_DIR}/gen.py --lang c --outdir ${DATALINK_OUT_DIR}
    DEPENDS ${DATALINK_GEN_SOURCES}
    COMMENT "Generating C protocol headers and sources..."
)

add_library(datalink STATIC
    ${DATALINK_OUT_DIR}/datalink.c
)
target_include_directories(datalink PUBLIC
    ${DATALINK_OUT_DIR}
)