idf_component_register(
    SRCS 
        "ds3231.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        rtc
        i2c-main
)