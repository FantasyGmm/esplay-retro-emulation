idf_component_register(SRC_DIRS "." "cpu" "sound"
        INCLUDE_DIRS "." "cpu" "sound"
        REQUIRES spiffs sdmmc vfs fatfs esplay-hal osd-menu esplay-ui
        )
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)
target_compile_options(${COMPONENT_LIB} PRIVATE -DLSB_FIRST=1)