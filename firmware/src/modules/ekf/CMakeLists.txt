set(DERIVATION_DIR ${CMAKE_CURRENT_LIST_DIR}/derivation)
set(EQS_OUT_DIR ${DERIVATION_DIR}/generated)
set(GENERATED_FILES
    ${EQS_OUT_DIR}/baro_fusion.h
    ${EQS_OUT_DIR}/covariance_prediction.h
    ${EQS_OUT_DIR}/gps_fusion_pos_d.h
    ${EQS_OUT_DIR}/gps_fusion_pos_e.h
    ${EQS_OUT_DIR}/gps_fusion_pos_n.h
    ${EQS_OUT_DIR}/gps_fusion_vel_d.h
    ${EQS_OUT_DIR}/gps_fusion_vel_e.h
    ${EQS_OUT_DIR}/gps_fusion_vel_n.h
)

file(MAKE_DIRECTORY ${EQS_OUT_DIR})
file(GLOB GEN_SOURCES CONFIGURE_DEPENDS ${DERIVATION_DIR}/*.py)

add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND Python3::Interpreter ${DERIVATION_DIR}/derivation.py
    DEPENDS ${GEN_SOURCES}
    COMMENT "Generating equtions for EKF model..."
)

add_module(ekf
    SOURCES
    EKFModule.cpp
    EKF.cpp
    ${GENERATED_FILES}
    DEPENDS
    maths
    geo
)