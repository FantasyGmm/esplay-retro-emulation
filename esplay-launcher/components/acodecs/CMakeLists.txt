idf_component_register(SRC_DIRS "src"
        INCLUDE_DIRS "include")

set(BUILD_STATIC ON)

function(install)
endfunction()

function(export)
endfunction()

add_subdirectory(src/libxmplite)

target_link_libraries(${COMPONENT_LIB} PRIVATE xmp_lite_static)

target_compile_options(${COMPONENT_LIB} PRIVATE -DLIBXMP_CORE_PLAYER -DVGM_YM2612_NUKED)