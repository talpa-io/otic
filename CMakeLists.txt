cmake_minimum_required(VERSION 3.11)

option(ENABLE_OTIC_TESTS "Enable Test of the C Otic lib" ON)

# TODO: ADD POLICY

project(otic
        VERSION 1.0.0
        DESCRIPTION "Open Telemetry Interchange Container Format"
        LANGUAGES C
        )


set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

configure_file(
        ${PROJECT_SOURCE_DIR}/include/config/config.h.in
        ${PROJECT_SOURCE_DIR}/include/config/config.h
)

if (ENABLE_OTIC_TESTS)
    add_subdirectory(tests)
endif()