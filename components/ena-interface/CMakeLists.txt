idf_component_register(
    SRCS 
        "ena-interface.c"
        "ena-interface-datetime.c"
        "ena-interface-menu.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        ena
)