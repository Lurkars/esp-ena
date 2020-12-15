idf_component_register(
    SRCS 
        "m5-input.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        interface
        imu-m5-mpu6886
)