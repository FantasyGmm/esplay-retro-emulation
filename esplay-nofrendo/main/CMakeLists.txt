idf_component_register(SRCS "main.c"
        REQUIRES spiffs sdmmc vfs fatfs esplay-hal
        )