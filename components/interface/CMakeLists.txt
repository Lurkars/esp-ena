set(priv_requires "ena" "ena-eke-proxy" "display" "rtc" "wifi-controller" )

if(CONFIG_ENA_INTERFACE_CUSTOM)
     list(APPEND priv_requires "display-custom-ssd1306" "rtc-custom-ds3231") 
elseif(CONFIG_ENA_INTERFACE_M5STICKC)
    list(CONFIG_APPEND priv_requires "display-m5-st7735s" "rtc-m5-bm8563" "imu-m5-mpu6886" "pmu-m5-axp192")
elseif(CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
    list(APPEND priv_requires "display-m5-st7789" "rtc-m5-bm8563" "imu-m5-mpu6886" "pmu-m5-axp192")
elseif(CONFIG_ENA_INTERFACE_TTGO_T_WRISTBAND)
    list(APPEND priv_requires "display-ttgo-st7735" "imu-ttgo-lsm9ds1")
else()   
    list(APPEND priv_requires "display-custom-ssd1306" "rtc-custom-ds3231") # uncomment for custom device with SSD1306 und DS3231
#    list(APPEND priv_requires "display-m5-st7735s" "rtc-m5-bm8563" "imu-m5-mpu6886" "pmu-m5-axp192") # uncomment for M5StickC
#    list(APPEND priv_requires "display-m5-st7789" "rtc-m5-bm8563" "imu-m5-mpu6886" "pmu-m5-axp192") # uncomment for M5StickC PLUS
#    list(APPEND priv_requires "display-ttgo-st7735" "imu-ttgo-lsm9ds1") # uncomment for TTGO T-Wristband
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