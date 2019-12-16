set(OTIC_CORE_FILES
        ${PROJECT_SOURCE_DIR}/include/core/base.h
        ${PROJECT_SOURCE_DIR}/include/core/config.h
        ${PROJECT_SOURCE_DIR}/include/core/pack.h
        ${PROJECT_SOURCE_DIR}/include/core/unpack.h
        ${PROJECT_SOURCE_DIR}/src/core/base.c
        ${PROJECT_SOURCE_DIR}/src/core/pack.c
        ${PROJECT_SOURCE_DIR}/src/core/unpack.c
        )

include(GenerateExportHeader)

add_library(otic_shared SHARED ${OTIC_CORE_FILES})
add_library(otic_static STATIC ${OTIC_CORE_FILES})
target_include_directories(otic_shared PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_include_directories(otic_static PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries(otic_shared zstd)
target_link_libraries(otic_static zstd)

set_target_properties(
        otic_shared
        PROPERTIES OUTPUT_NAME otic
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

set_target_properties(
        otic_static
        PROPERTIES OUTPUT_NAME otic
)

target_compile_options(otic_shared PRIVATE -Wall -Wextra -Wpedantic)

generate_export_header(otic_shared)
include(GNUInstallDirs)


install(TARGETS otic_shared
        EXPORT oticTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
install(TARGETS otic_static
        EXPORT oticTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/oticConfigVersion.cmake"
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        COMPATIBILITY AnyNewerVersion
)

export(EXPORT oticTargets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/oticTargets.cmake
        NAMESPACE otic::
        )

set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
set(SYSCONFIG_INSTALL_DIR etc/otic/)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/oticConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/oticConfig.cmake
        INSTALL_DESTINATION ${LIB_INSTALL_DIR}/otic/cmake
        PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR
        )

set(ConfigPackageLocation lib/cmake/otic)
install(EXPORT oticTargets
        FILE oticTargets.cmake
        NAMESPACE otic::
        DESTINATION ${ConfigPackageLocation}
        )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/oticConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/oticConfigVersion.cmake"
        DESTINATION ${ConfigPackageLocation}
        COMPONENT Devel
        )

set(OTIC_CORE_FILES ${OTIC_CORE_FILES} PARENT_SCOPE)