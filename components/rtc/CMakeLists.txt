set(include_list ".")

if(CONFIG_ENA_INTERFACE_RTC_DS3231)
    set(src_list "custom-ds3231/ds3231.c")
    list(APPEND include_list "custom-ds3231")
elseif(CONFIG_ENA_INTERFACE_M5STICKC OR CONFIG_ENA_INTERFACE_M5STICKC_PLUS)
    set(src_list "m5-bm8563/bm8563.c")
    list(APPEND include_list "m5-bm8563")
else()
    set(src_list "dummy.c")
endif()

idf_component_register(
    SRCS 
        ${src_list}
    INCLUDE_DIRS
        ${include_list}
    PRIV_REQUIRES
        "i2c-main"
)