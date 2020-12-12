set(priv_requires "ena" "ena-eke-proxy" "display" "rtc" "wifi-controller" )

if(CONFIG_ENA_INTERFACE_CUSTOM)
     list(APPEND priv_requires "display-ssd1306" "rtc-ds3231") 
elseif(ENA_INTERFACE_M5STICKC)
    list(APPEND priv_requires "display-st7735s" "rtc-bm8563" "imu-mpu6886" "pmu-axp192")
elseif(ENA_INTERFACE_M5STICKC_PLUS)
    list(APPEND priv_requires "display-st7789" "rtc-bm8563" "imu-mpu6886" "pmu-axp192")
else()   
   list(APPEND priv_requires "display-ssd1306" "rtc-ds3231") # uncomment for custom device with SSD1306 und DS3231
#    list(APPEND priv_requires "display-st7735s" "rtc-bm8563" "imu-mpu6886" "pmu-axp192") # uncomment for M5StickC
#    list(APPEND priv_requires "display-st7789" "rtc-bm8563" "imu-mpu6886" "pmu-axp192") # uncomment for M5StickC PLUS
endif()

idf_component_register(
    SRCS 
        "interface.c"
        "interface-main.c"
        "interface-data.c"
        "interface-datetime.c"
        "interface-debug.c"
        "interface-info.c"
        "interface-input.c"
        "interface-label.c"
        "interface-report.c"
        "interface-settings.c"
        "interface-wifi.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES ${priv_requires}
)