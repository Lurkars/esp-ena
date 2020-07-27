idf_component_register(
    SRCS 
        "interface.c"
        "interface-datetime.c"
        "interface-menu.c"
        "interface-status.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        ena
)