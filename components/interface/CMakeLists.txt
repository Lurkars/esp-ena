idf_component_register(
    SRCS 
        "interface.c"
        "interface-main.c"
        "interface-data.c"
        "interface-datetime.c"
        "interface-input.c"
        "interface-label.c"
        "interface-report.c"
        "interface-settings.c"
        "interface-wifi.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES 
        ena
        ssd1306
        ds3231
        wifi-controller
)