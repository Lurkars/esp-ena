idf_component_register(
    SRCS 
        "st7735s.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        "display"
        "spi_flash"
        "pmu-m5-axp192"
)
