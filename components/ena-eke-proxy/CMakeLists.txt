idf_component_register(
    SRCS 
        "ena-eke-proxy.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        esp_http_client
        ena
        wifi-controller
    EMBED_FILES
        "certs/cert.pem"
)