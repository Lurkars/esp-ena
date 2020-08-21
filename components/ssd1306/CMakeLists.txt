idf_component_register(
    SRCS 
        "ssd1306.c"
        "ssd1306-gfx.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        i2c-main
)