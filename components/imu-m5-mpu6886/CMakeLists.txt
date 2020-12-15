idf_component_register(
    SRCS 
        "mpu6886.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        i2c-main
)