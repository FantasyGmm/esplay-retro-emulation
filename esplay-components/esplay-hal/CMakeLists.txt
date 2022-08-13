idf_component_register(SRC_DIRS "src"
        INCLUDE_DIRS "include"
        REQUIRES sdmmc vfs fatfs nvs_flash esp_adc_cal app_update spiffs)