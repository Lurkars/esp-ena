idf_component_register(
    SRCS 
        "ds3231.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        i2c-main
)