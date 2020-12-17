set(src_list "display.c" "display-gfx.c")
set(include_list ".")

if(CONFIG_ENA_INTERFACE_DISPLAY_SSD1306)
     list(APPEND src_list "custom-ssd1306/ssd1306.c")
     list(APPEND include_list "custom-ssd1306")
elseif(CONFIG_ENA_INTERFACE_M5STICKC)
    list(APPEND src_list "m5-st7735s/st7735s.c" "m5-axp192/axp192.c")
    list(APPEND include_list "m5-st7735s" "m5-axp192")
elseif(CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
    list(APPEND src_list "m5-st7789/st7789.c" "m5-axp192/axp192.c")
    list(APPEND include_list "m5-st7789" "m5-axp192")
elseif(CONFIG_ENA_INTERFACE_TTGO_T_WRISTBAND)
    list(APPEND src_list "ttgo-st7735/st7735.c")
    list(APPEND include_list "ttgo-st7735")
else()
    list(APPEND src_list "dummy.c")
endif()

idf_component_register(
    SRCS 
        ${src_list}
    INCLUDE_DIRS
        ${include_list}
    PRIV_REQUIRES
        "i2c-main"
        "spi_flash"
)