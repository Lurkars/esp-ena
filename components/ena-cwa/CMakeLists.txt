idf_component_register(
    SRCS 
        "ena-cwa.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        esp_http_client
        miniz
        ena
        wifi-controller
    EMBED_FILES
        "certs/telekom.pem"
)