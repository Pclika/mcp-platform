idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES pclika_runtime pclika_bridge nvs_flash driver
)
