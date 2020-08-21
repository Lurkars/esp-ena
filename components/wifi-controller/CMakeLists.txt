idf_component_register(
    SRCS 
        "wifi-controller.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        esp_wifi
        nvs_flash
)