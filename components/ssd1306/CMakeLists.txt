idf_component_register(
    SRCS 
        "ssd1306.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        i2c-main
)