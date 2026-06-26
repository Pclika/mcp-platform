idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        pclika_runtime
        driver
        nvs_flash
        cJSON
        esp_adc_cal
)
