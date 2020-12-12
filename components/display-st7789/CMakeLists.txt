idf_component_register(
    SRCS 
        "st7789.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        "display"
        "spi_flash"
        "pmu-axp192"
)