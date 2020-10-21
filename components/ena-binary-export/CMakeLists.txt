idf_component_register(
    SRCS 
        "ena-binary-export.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        ena
        nanopb
    EMBED_FILES
        "test/export.bin"
)