idf_component_register(
    SRCS 
        "bm8563.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        rtc 
        i2c-main
)