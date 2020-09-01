cmake_minimum_required(VERSION 3.8)

cmake_policy(PUSH)

if (POLICY CMP0048)
    message(STATUS "To newer CMP0048 Policy")
    cmake_policy(SET CMP0048 NEW)
endif()

if (POLICY CMP0054)
    message(STATUS "To newer CMP0054 Policy")
    cmake_policy(SET CMP0054 NEW)
endif()


set(CMAKE_C_STANDARD 99)

option(ALLOW_CMAKE_LIB_FETCH "Allow CMake to Fetch the missing libs from the Web" ON)
option(ENABLE_TESTS "Enable tests of the core OTIC library" OFF)
option(ENABLE_FUZZ "Enable Build of the Fuzzing of the OTIC library" OFF)
option(BUILD_DEMOS "Build demos and examples" ON)
option(PHP_BIND "Build PHP Bindings" OFF)
option(OTIC_PACK_NO_COMPRESSION "Disable Otic Pack Second Compression: ZSTD, ZLIB, GZIP" OFF)
option(OTIC_PACK_INLINE_ALL_STATIC "Inline every static functions in the Otic Pack src file" ON)
option(OTIC_UNPACK_INLINE_ALL_STATIC "Inline every static functions in the Otic Unpack src file" ON)
option(OTIC_STATS "Enable Statistics" ON)
set(PACK_BUCKET_SIZE 3276800 CACHE STRING "Cache size in bytes of the pack bucket")


if (ENABLE_FUZZ)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang)
endif()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting the build type to: Release")
    set(CMAKE_BUILD_TYPE Release)
else()
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
endif()

project(otic
        VERSION 2.0.0
        DESCRIPTION "Open Telemetry Interchange Container Format"
        LANGUAGES C CXX
        )


if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
link_directories(${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

include(cmake/tools.cmake)
include(TestBigEndian)
if (${CMAKE_VERSION} VERSION_GREATER "3.11")
    include(cmake/dependencies.cmake)
endif()

TEST_BIG_ENDIAN(BIG_ENDIAN_SYS)
CHECK_INCLUDE_FILE(endian.h HAS_ENDIAN_H)

configure_file(
        ${PROJECT_SOURCE_DIR}/include/core/config.h.in
        ${PROJECT_SOURCE_DIR}/include/core/config.h
)

CHECK_INCLUDE_FILE(zstd.h HAS_ZSTD_H)

if (NOT HAS_ZSTD_H)
    if (ALLOW_CMAKE_LIB_FETCH)
        if (${CMAKE_VERSION} VERSION_LESS "3.11")
            message(WARNING "Can't fetch missing zstd from github, as your CMake's Version: ${CMAKE_VERSION} doesn't support content fetch")
        else()
            set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Disable Zstd programs build")
            include(FetchContent)
            FetchContent_GetProperties(zstd)
            if (NOT zstd_POPULATED)
                FetchContent_Populate(zstd)
                add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})
            endif()
            include_directories(${zstd_SOURCE_DIR}/lib)
        endif()
    endif(ALLOW_CMAKE_LIB_FETCH)

#    set(LIBRARY_DIR ${zstd_SOURCE_DIR}/lib)
#    file(GLOB CommonSources ${LIBRARY_DIR}/common/*.c)
#    file(GLOB CompressSources ${LIBRARY_DIR}/compress/*.c)
#    file(GLOB DecompressSources ${LIBRARY_DIR}/decompress/*.c)
#    file(GLOB DictBuilderSources ${LIBRARY_DIR}/dictBuilder/*.c)
#    file(GLOB DeprecatedSources ${LIBRARY_DIR}/deprecated/*.c)
    #
#    set(Sources
#            ${CommonSources}
#            ${CompressSources}
#            ${DecompressSources}
#            ${DictBuilderSources}
#            ${DeprecatedSources})

#    file(GLOB CommonHeaders ${LIBRARY_DIR}/common/*.h)
#    file(GLOB CompressHeaders ${LIBRARY_DIR}/compress/*.h)
#    file(GLOB DecompressHeaders ${LIBRARY_DIR}/decompress/*.h)
#    file(GLOB DictBuilderHeaders ${LIBRARY_DIR}/dictBuilder/*.h)
#    file(GLOB DeprecatedHeaders ${LIBRARY_DIR}/deprecated/*.h)

#    set(Headers
#            ${LIBRARY_DIR}/zstd.h
#            ${CommonHeaders}
#            ${CompressHeaders}
#            ${DecompressHeaders}
#            ${DictBuilderHeaders}
#            ${DeprecatedHeaders}
#            )
#    include_directories(${LIBRARY_DIR}/common ${LIBRARY_DIR}/compress ${LIBRARY_DIR}/decompress ${LIBRARY_DIR}/dicBuilder ${LIBRARY_DIR}/deprecated)
#    message(STATUS "Sources: ${zstd_SOURCE_DIR}")
else()
    message(STATUS "ZSTD found")
endif()

add_subdirectory(src)

if (ENABLE_TESTS)
    include(CTest)
    find_package(unity QUIET)
    if (${unity_FOUND})
        set(unity_SOURCE_DIR ${CMAKE_INSTALL_PREFIX})
        message(STATUS "Unity found!")
    else()
        message(STATUS "Fetching Unity")
        if (${CMAKE_VERSION} VERSION_LESS "3.11")
            message(WARNING "Content Fetching not supported to fetch Unity from Github with your current CMake version: ${CMAKE_VERSION}. Required: At least Version 3.11")
        else()
            include(FetchContent)
            FetchContent_GetProperties(unity)
            if (NOT unity_POPULATED)
                FetchContent_Populate(unity)
                add_subdirectory(${unity_SOURCE_DIR} ${unity_BINARY_DIR})
                include_directories(${unity_SOURCE_DIR}/src)
            endif()
        endif()
    endif()
    add_subdirectory(test/ctest)
endif()

if (ENABLE_FUZZ)
    add_subdirectory(test/fuzz)
endif()

if (BUILD_DEMOS)
    add_subdirectory(demo)
endif()

if (PHP_BIND)
    add_subdirectory(bindings/php7)
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
        EXPORT oticTargets
        DESTINATION include/otic
        FILES_MATCHING PATTERN "*.h"
        )

cmake_policy(POP)
