idf_component_register(
    SRCS 
        "ssd1306.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        "display"
        "i2c-main"
)