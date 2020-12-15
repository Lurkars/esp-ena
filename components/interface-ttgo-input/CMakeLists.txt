idf_component_register(
    SRCS 
        "ttgo-input.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        interface
        imu-ttgo-lsm9ds1
)