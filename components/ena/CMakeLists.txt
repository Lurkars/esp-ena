idf_component_register(
    SRCS 
        "beacons.c"
        "bluetooth-advertise.c"
        "bluetooth-scan.c"
        "crypto.c"
        "ena.c"
        "storage.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        spi_flash
        mbedtls
        bt
)