set(EQS_OUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
set(GENERATED_FILES
    ${EQS_OUT_DIR}/baro_fusion.c
    ${EQS_OUT_DIR}/covariance_prediction.c
    ${EQS_OUT_DIR}/gps_fusion_pos_d.c
    ${EQS_OUT_DIR}/gps_fusion_pos_e.c
    ${EQS_OUT_DIR}/gps_fusion_pos_n.c
    ${EQS_OUT_DIR}/gps_fusion_vel_d.c
    ${EQS_OUT_DIR}/gps_fusion_vel_e.c
    ${EQS_OUT_DIR}/gps_fusion_vel_n.c
)

file(MAKE_DIRECTORY ${EQS_OUT_DIR})
file(GLOB GEN_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.py)

add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND Python3::Interpreter ${CMAKE_CURRENT_LIST_DIR}/derivation.py
    DEPENDS ${GEN_SOURCES}
    COMMENT "Generating equtions for EKF model..."
)

add_custom_target(EKF_Derivation_ForceGeneration ALL DEPENDS ${GENERATED_FILES})