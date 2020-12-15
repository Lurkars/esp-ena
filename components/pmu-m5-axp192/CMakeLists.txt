idf_component_register(
    SRCS 
        "axp192.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        i2c-main
)