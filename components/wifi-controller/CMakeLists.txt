idf_component_register(
    SRCS 
        "espidf-v4.1.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        esp_wifi
        nvs_flash
)
