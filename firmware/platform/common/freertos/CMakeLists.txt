add_library(freertos_osal INTERFACE)

target_sources(freertos_osal INTERFACE
    src/task_freertos.c
)

target_link_libraries(freertos_osal INTERFACE
    platform_osal
    FreeRTOS-Kernel-Static
)