idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        pclika_runtime
        esp_camera
        nvs_flash
        mbedtls
        cJSON
)
