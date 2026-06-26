idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        pclika_runtime
        esp_wifi
        nvs_flash
        esp_event
        cJSON
)
