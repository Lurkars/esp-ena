set(src_list 
    "interface.c"
    "interface-main.c"
    "interface-data.c"
    "interface-datetime.c"
    "interface-info.c"
    "interface-input.c"
    "interface-label.c"
    "interface-report.c"
    "interface-settings.c"
    "interface-wifi.c")

set(include_list ".")

if(CONFIG_ENA_INTERFACE_INPUT_CUSTOM)
     list(APPEND src_list "custom-input/custom-input.c")
     list(APPEND include_list "custom-input")
elseif(CONFIG_ENA_INTERFACE_M5STICKC OR CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
    list(APPEND src_list "m5-input/m5-input.c" "m5-mpu6886/mpu6886.c")
    list(APPEND include_list "m5-input" "m5-mpu6886")
elseif(CONFIG_ENA_INTERFACE_TTGO_T_WRISTBAND)
    list(APPEND src_list "ttgo-input/ttgo-input.c" "ttgo-lsm9ds1/lsm9ds1.c")
    list(APPEND include_list "ttgo-input" "ttgo-lsm9ds1")
else()
    list(APPEND src_list "dummy.c")
endif()

idf_component_register(
    SRCS 
        ${src_list}
    INCLUDE_DIRS
        ${include_list}
    PRIV_REQUIRES 
        "ena"
        "ena-eke-proxy"
        "display"
        "rtc"
        "wifi-controller"
        "i2c-main"
)
