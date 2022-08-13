idf_component_register(SRC_DIRS "." "cpu" "libsnss" "nes" "sndhrdw" "mappers"
        INCLUDE_DIRS "." "cpu" "libsnss" "nes" "sndhrdw"
        REQUIRES spiffs sdmmc vfs fatfs esplay-hal osd-menu esplay-ui nofrendo-esp32
        )
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=char-subscripts" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=attributes" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Werror=stringop-truncation" APPEND)