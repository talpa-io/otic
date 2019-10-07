cmake_minimum_required(VERSION 3.11)

option(ALLOW_CMAKE_LIB_FETCH "Allow CMake to Fetch the missing libs from the Web" ON)
option(ENABLE_OPTIC_TESTS "Enable tests of the core otic" ON)
option(BUILD_DEMOS "Build demos and examples" ON)

# TODO: ADD POLICY

project(otic
        VERSION 1.0.0
        DESCRIPTION "Timestamp Row Format"
        LANGUAGES C
        )

set(CMAKE_C_STANDARD 99)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/lib)


configure_file(
        ${PROJECT_SOURCE_DIR}/include/config/config.h.in
        ${PROJECT_SOURCE_DIR}/include/config/config.h
)

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

add_subdirectory(src)

if (ENABLE_OPTIC_TESTS)
    add_subdirectory(tests)
endif()

if (BUILD_DEMOS)
    add_subdirectory(demo)
endif()

add_subdirectory(bindings/php)