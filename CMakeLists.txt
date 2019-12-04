cmake_minimum_required(VERSION 3.10)

if (POLICY CMP0048)
    message(STATUS "To newer CMP0048 Policy")
    cmake_policy(SET CMP0048 NEW)
endif()

if (POLICY CMP0054)
    message(STATUS "To newer CMP0054 Policy")
    cmake_policy(SET CMP0054 NEW)
endif()

option(ALLOW_CMAKE_LIB_FETCH "Allow CMake to Fetch the missing libs from the Web" ON)
option(ENABLE_OPTIC_TESTS "Enable tests of the core otic" ON)
option(BUILD_DEMOS "Build demos and examples" ON)
option(PHP_BIND "Build PHP Bindings" OFF)
option(OTIC_PACK_NO_COMPRESSION "Disable Otic Pack Second Compression: ZSTD, ZLIB, GZIP" OFF)
option(OTIC_PACK_INLINE_ALL_STATIC "Inline every static functions in the Otic Pack src file" ON)

#if (NOT ${CMAKE_BUILD_TYPE})
#    message(STATUS "Setting the build type to: Release")
#    set(CMAKE_BUILD_TYPE Release)
#else()
#    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
#endif()

project(otic
        VERSION 1.0.0
        DESCRIPTION "Open Telemetry Interchange Container Format"
        LANGUAGES C
        )

set(CMAKE_C_STANDARD 99)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/lib)

configure_file(
        ${PROJECT_SOURCE_DIR}/include/core/config.h.in
        ${PROJECT_SOURCE_DIR}/include/core/config.h
)

include(cmake/tools.cmake)

CHECK_INCLUDE_FILE(zstd.h ZSTD_EXISTS)

if (NOT ZSTD_EXISTS)
    if (ALLOW_CMAKE_LIB_FETCH)
        set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Disable Zstd programs build")
        include(FetchContent)
    #    set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/deps)
        FetchContent_Declare(
                zstd
                GIT_REPOSITORY https://github.com/facebook/zstd.git
                GIT_TAG        v1.4.3
        )
        FetchContent_GetProperties(zstd)
        if (NOT zstd_POPULATED)
            FetchContent_Populate(zstd)
            add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})
        endif()
        include_directories(${zstd_SOURCE_DIR}/lib)
    endif(ALLOW_CMAKE_LIB_FETCH)
endif()

add_subdirectory(src)

if (ENABLE_OPTIC_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()

if (BUILD_DEMOS)
    add_subdirectory(demo)
endif()

if (PHP_BIND)
    add_subdirectory(bindings/php)
endif()

FILE(WRITE
    "${PROJECT_BINARY_DIR}/otic.h"
    "#ifndef OTIC_H\n#define OTIC_H\n\n#include <otic/otic.h>\n\n#endif // OTIC_H"
    )

install(FILES "${PROJECT_BINARY_DIR}/otic.h"
        CONFIGURATIONS Release | Debug
        DESTINATION include
        COMPONENT Devel
        )

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include/otic
        FILES_MATCHING PATTERN "*.h"
        )

#message(STATUS "No_compression: ${OTIC_PACK_NO_COMPRESSION}")

#install(TARGETS libotic
#        CONFIGURATIONS Release | Debug
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib
#        RUNTIME DESTINATION bin
#        INCLUDES DESTINATION include
#        COMPONENT Libraries
#        NAMELINK_COMPONENT Developement
#        PUBLIC_HEADER
#        DESTINATION include
#        COMPONENT Development
#        )

#install(DIRECTORY src/
#        DESTINATION include/otic
#        FILES_MATCHING PATTERN "*.[h,c]"
#        )
#
#install(DIRECTORY include/config/
#        DESTINATION include/otic
#        FILES_MATCHING PATTERN "*.[h]"
#        )

#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#        ${CMAKE_CURRENT_BINARY_DIR}/otic/OticConfigVersion.cmake
#        VERSION ${PROJECT_VERSION_MAJOR}
#        COMPATIBILITY AnyNewerVersion
#)
#
#export(EXPORT lib_otic
#        FILE "${CMAKE_CURRENT_BINARY_DIR}/otic/OticTargets.cmake"
#        NAMESPACE otic::)
#
#configure_file(cmake/OticConfig.cmake
#        "${CMAKE_CURRENT_BINARY_DIR}/otic/OticConfig.cmake"
#        COPYONLY
#        )
#
#set(ConfigPackageLocation lib/cmake/Otic)
#install(EXPORT lib_otic
#        FILE OticTargets.cmake
#        NAMESPACE Otic::
#        DESTINATION ${ConfigPackageLocation}
#        )
#
#install(FILES
#        cmake/OticConfig.cmake
#        "${CMAKE_CURRENT_BINARY_DIR}/otic/OticConfigVersion.cmake"
#        DESTINATION ${ConfigPackageLocation}
#        COMPONENT Devel
#        )

#export(TARGETS otic
#        FILE "${PROJECT_BINARY_DIR}/otic.cmake")
#
#export(PACKAGE otic)

#file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
#set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
#configure_file("${PROJECT_SOURCE_DIR}/cmake/OticConfig.cmake.in" "${PROJECT_BINARY_DIR}/OticConfig.cmake" @ONLY)

#set(CONF_INCLUDE_DIRS "\${}")
#message(STATUS ${REL_INCLUDE_DIR})