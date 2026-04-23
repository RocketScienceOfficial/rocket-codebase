idf_component_register(SRCS
    "main.cpp"
    "hal/adc_driver_esp32.c"
    "hal/flash_driver_esp32.c"
    "hal/gpio_driver_esp32.c"
    "hal/i2c_driver_esp32.c"
    "hal/pwm_driver_esp32.c"
    "hal/spi_driver_esp32.c"
    "hal/stdio_driver_esp32.c"
    "hal/time_driver_esp32.c"
    "hal/uart_driver_esp32.c"
    "hal/ws2812b_driver_esp32.c"
    "osal/systime_esp32.c"
    "osal/task_esp32.c"

    INCLUDE_DIRS "."

    REQUIRES
    driver
    freertos
    esp_driver_uart
    esp_driver_spi
    esp_driver_i2c
    esp_driver_gpio
    esp_driver_ledc
    esp_adc
    spi_flash
    esp_timer

    WHOLE_ARCHIVE
)

target_link_libraries(${COMPONENT_LIB} PRIVATE
    platform_hal
    platform_osal
    app_main
)