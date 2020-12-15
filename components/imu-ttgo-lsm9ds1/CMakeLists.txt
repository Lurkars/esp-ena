idf_component_register(
    SRCS 
        "lsm9ds1.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        i2c-main
)