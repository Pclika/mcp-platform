idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        pclika_runtime
        pclika_servo
        pclika_bridge
        esp_timer
        freertos
        cJSON
)
