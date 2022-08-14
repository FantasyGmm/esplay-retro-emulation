idf_component_register(SRCS "main.c" "event.c" "file_ops.c" "graphics.c" "audio_player.c"
                    INCLUDE_DIRS "include")
add_compile_options(-Wno-format-truncation)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)