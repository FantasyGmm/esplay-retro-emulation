idf_component_register(SRCS src/uri_encode.c src/file_server.c
    INCLUDE_DIRS include
    EMBED_FILES src/favicon.ico src/upload_script.html
    REQUIRES sdmmc vfs fatfs esp_http_server
        )