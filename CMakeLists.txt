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
option(ENABLE_TESTS "Enable tests of the core OTIC library" ON)
option(ENABLE_FUZZ "Enable Build of the Fuzzing of the OTIC library" OFF)
option(BUILD_DEMOS "Build demos and examples" ON)
option(PHP_BIND "Build PHP Bindings" OFF)
option(OTIC_PACK_NO_COMPRESSION "Disable Otic Pack Second Compression: ZSTD, ZLIB, GZIP" OFF)
option(OTIC_PACK_INLINE_ALL_STATIC "Inline every static functions in the Otic Pack src file" ON)
option(OTIC_UNPACK_INLINE_ALL_STATIC "Inline every static functions in the Otic Unpack src file" ON)
set(PACK_BUCKET_SIZE 16384 CACHE STRING "Cache size in bytes of the pack bucket")

if (ENABLE_FUZZ)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang)
endif()


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

include(cmake/tools.cmake)
include(cmake/dependencies.cmake)
include(TestBigEndian)

TEST_BIG_ENDIAN(BIG_ENDIAN_SYS)
CHECK_INCLUDE_FILE(endian.h HAS_ENDIAN_H)

configure_file(
        ${PROJECT_SOURCE_DIR}/include/core/config.h.in
        ${PROJECT_SOURCE_DIR}/include/core/config.h
)

CHECK_INCLUDE_FILE(zstd.h HAS_ZSTD_H)

find_package(zstd QUIET)
if (zstd_FOUND)
    message(STATUS "Zstd found")
else()
    message(STATUS "ZSTD not found")
endif()


if (NOT HAS_ZSTD_H)
    if (ALLOW_CMAKE_LIB_FETCH)
        set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Disable Zstd programs build")
        include(FetchContent)
        FetchContent_GetProperties(zstd)
        if (NOT zstd_POPULATED)
            FetchContent_Populate(zstd)
            add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})
        endif()
        include_directories(${zstd_SOURCE_DIR}/lib)
    endif(ALLOW_CMAKE_LIB_FETCH)
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
        FetchContent_GetProperties(unity)
        if (NOT unity_POPULATED)
            FetchContent_Populate(unity)
            add_subdirectory(${unity_SOURCE_DIR} ${unity_BINARY_DIR})
            include_directories(${unity_SOURCE_DIR}/src)
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
