idf_component_register(SRC_DIRS "."
        INCLUDE_DIRS "."
        REQUIRES spiffs sdmmc vfs fatfs esplay-hal
        )
idf_build_set_property(COMPILE_OPTIONS "-Werror=array-bounds" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wdiscarded-qualifiers" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wunused-function" APPEND)
target_compile_options(${COMPONENT_LIB} PRIVATE -DGNUBOY_NO_MINIZIP -DGNUBOY_NO_SCREENSHOT -DIS_LITTLE_ENDIAN)