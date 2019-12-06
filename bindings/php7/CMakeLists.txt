cmake_minimum_required(VERSION 3.10)

project(
        otic_php
        VERSION 1.0.0
        DESCRIPTION "OTIC PHP binding"
        LANGUAGES C
)

if (${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_HOME_DIRECTORY})
    message(STATUS "Building PHP_OTIC independantly")
#        TODO: ADD FIND_PACKAGE AND FETCH INCLUDE_DIR
    set(OTIC_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/../../include)
else()
    set(OTIC_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/include)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_MODUlE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_C_FLAGS "-g -O2 -DHAVE_CONFIG_H")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

execute_process(
        COMMAND php-config --include-dir
        OUTPUT_VARIABLE PHP_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (PHP_INCLUDE_DIR)
    message(STATUS "PHP Include DIR found at: ${PHP_INCLUDE_DIR}")
else(PHP_INCLUDE_DIR)
    message(FATAL_ERROR "PHP Include DIR not found")
endif()

execute_process(
        COMMAND php-config --extension-dir
        OUTPUT_VARIABLE PHP_EXTENSION_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (PHP_EXTENSION_DIR)
    message(STATUS "PHP Extension DIR found at: ${PHP_EXTENSION_DIR}")
else(PHP_EXTENSION_DIR)
    message(FATAL_ERROR "PHP Extension DIR not found")
endif()

execute_process(
        COMMAND php-config --includes
        OUTPUT_VARIABLE PHP_INCLUDES
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "-I" "" PHP_INCLUDES ${PHP_INCLUDES})
separate_arguments(PHP_INCLUDES)
message(STATUS "PHP Including: ")
foreach (X ${PHP_INCLUDES})
    message(STATUS "\t${X}")
endforeach ()

include(cmake/php.cmake)

file(GLOB OTIC_PHP_INCLUDE include/*h)
file(GLOB OTIC_PHP_SRC src/*.c)

set(OTIC_CORE_FILES
        ${PROJECT_SOURCE_DIR}/../../include/core/base.h
        ${PROJECT_SOURCE_DIR}/../../include/core/config.h
        ${PROJECT_SOURCE_DIR}/../../include/core/pack.h
        ${PROJECT_SOURCE_DIR}/../../include/core/unpack.h
        ${PROJECT_SOURCE_DIR}/../../src/core/base.c
        ${PROJECT_SOURCE_DIR}/../../src/core/pack.c
        ${PROJECT_SOURCE_DIR}/../../src/core/unpack.c
        ${PROJECT_SOURCE_DIR}/../../src/utility/errHand.c
        ${PROJECT_SOURCE_DIR}/../../include/utility/errHand.h
        )

add_library(
        otic_php SHARED
        ${OTIC_PHP_INCLUDE}
        ${OTIC_PHP_SRC}
        ${OTIC_CORE_FILES}
)
target_include_directories(
        otic_php PRIVATE
        ${PHP_INCLUDES}
        ${PROJECT_SOURCE_DIR}/include
        ${OTIC_INCLUDE_DIR}
)
target_link_libraries(otic_php zstd)

add_php_executable(test/pack.php)
add_php_executable(test/unpack.php)