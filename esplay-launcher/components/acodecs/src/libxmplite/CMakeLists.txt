cmake_minimum_required(VERSION 3.2)
project(libxmplite VERSION 4.5.1 LANGUAGES C)

set(LIBXMP_DEFINES)
set(LIBXMP_DEFINES_PUBLIC)
set(LIBXMP_CFLAGS)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/libxmp-checks.cmake)
include(GNUInstallDirs)

# -fPIC thing
if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT VITA)
    option(LIBXMP_PIC   "Build the static library as position-independent" OFF)
else()
    set(LIBXMP_PIC OFF)
endif()
# -----------------------------------


option(BUILD_STATIC   "Build libXMP static library" ON)
if(NOT BUILD_STATIC)
    message(FATAL_ERROR "Both static and shared builds got disabled. You must enable at least one of them.")
endif()

option(LIBXMP_DISABLE_IT            "Disable IT format support in libXMP-lite" OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/libxmp-sources.cmake)

list(APPEND LIBXMP_DEFINES_PUBLIC -DLIBXMP_CORE_PLAYER)
if(LIBXMP_DISABLE_IT)
    list(APPEND LIBXMP_DEFINES_PUBLIC -DLIBXMP_CORE_DISABLE_IT)
endif()


set(XMP_INSTALLS)

if(BUILD_STATIC)
    add_library(xmp_lite_static STATIC ${LIBXMP_SRC_LIST})
    add_library(libxmp-lite::xmp_lite_static ALIAS xmp_lite_static)
    list(APPEND XMP_INSTALLS xmp_lite_static)
    set_target_properties(xmp_lite_static PROPERTIES C_STANDARD 90)
    if(MSVC)
        set_target_properties(xmp_lite_static PROPERTIES OUTPUT_NAME libxmp-lite-static)
    else()
        set_target_properties(xmp_lite_static PROPERTIES OUTPUT_NAME xmp-lite)
    endif()

    target_compile_options(xmp_lite_static PRIVATE ${LIBXMP_CFLAGS})
    target_compile_definitions(xmp_lite_static PRIVATE ${LIBXMP_DEFINES} -DBUILDING_STATIC)
    target_compile_definitions(xmp_lite_static PUBLIC ${LIBXMP_DEFINES_PUBLIC})

    if(LIBXMP_PIC)
        target_compile_options(xmp_lite_static PRIVATE -fPIC)
    endif()

    target_include_directories(xmp_lite_static PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/libxmp-lite>")
    target_include_directories(xmp_lite_static PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libxmp-lite>")

    if(LIBM_REQUIRED)
        target_link_libraries(xmp_lite_static PUBLIC ${LIBM_LIBRARY})
    endif()
endif()

add_library(XMP_IF INTERFACE)

target_link_libraries(XMP_IF INTERFACE xmp_lite_static)

target_compile_definitions(XMP_IF INTERFACE -DBUILDING_STATIC)
# === Unit tests ====
option(WITH_UNIT_TESTS "Enable unit testing" OFF)
if(WITH_UNIT_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build_examples.cmake")
    include(build_examples.cmake)
endif()


# === Install ====

include(CMakePackageConfigHelpers)

if(WIN32)
    set(cmake_install_cmakdir "cmake")
else()
    set(cmake_install_cmakdir "${CMAKE_INSTALL_LIBDIR}/cmake/libxmp-lite")
endif()

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(libdir     "${CMAKE_INSTALL_FULL_LIBDIR}")
set(bindir     "${CMAKE_INSTALL_FULL_BINDIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libxmp-lite.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/libxmp-lite.pc" @ONLY
)
write_basic_package_version_file(libxmp-lite-config-version.cmake
    COMPATIBILITY AnyNewerVersion
)
if(TARGET xmp_lite_static)
    install(TARGETS xmp_lite_static EXPORT libxmp_lite_static_exports
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(EXPORT libxmp_lite_static_exports
        DESTINATION "${cmake_install_cmakdir}"
        FILE "libxmp-lite-static-targets.cmake"
        NAMESPACE "libxmp-lite::"
    )
endif()
install(FILES
    libxmp-lite-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/libxmp-lite-config-version.cmake"
    DESTINATION "${cmake_install_cmakdir}"
)
install(FILES
        include/libxmp-lite/xmp.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libxmp-lite
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libxmp-lite.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
