idf_component_register(
    SRCS 
        "ena.c"
        "ena-beacons.c"
        "ena-bluetooth-advertise.c"
        "ena-bluetooth-scan.c"
        "ena-crypto.c"
        "ena-storage.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        spi_flash
        mbedtls
        bt
)