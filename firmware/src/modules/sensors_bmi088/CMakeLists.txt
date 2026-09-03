add_library(drivers_imu STATIC
    bmi088_acc_driver.c
    bmi088_gyro_driver.c
)

target_link_libraries(drivers_imu PUBLIC
    drivers_common
    maths
    geo
)