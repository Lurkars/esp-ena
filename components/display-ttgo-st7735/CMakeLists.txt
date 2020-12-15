idf_component_register(
    SRCS 
        "st7735.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        "display"
        "spi_flash"
)
