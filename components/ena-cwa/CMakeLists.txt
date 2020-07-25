idf_component_register(
    SRCS 
        "ena-cwa.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES
        esp_http_client
        miniz
        ena
    EMBED_FILES
        "certs/telekom.pem"
)