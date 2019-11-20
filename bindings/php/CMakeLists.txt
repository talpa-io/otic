cmake_minimum_required(VERSION 3.10)
project(
        php_otic
        VERSION 1.0.0
        DESCRIPTION "OTIC PHP binding"
        LANGUAGES C
)

set(CMAKE_MODUlE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0 -Wall -std=gnu99 -fvisibility=hidden")
set(CMAKE_C_FLAGS "-g -O2 -DHAVE_CONFIG_H")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

include(cmake/php.cmake)
include(CheckIncludeFile)

CHECK_INCLUDE_FILE(otic.h HAS_OTIC_H CMAKE_REQUIRED_INCLUDES)
#if (NOT HAS_OTIC_H)
#    message(FATAL_ERROR "otic.h was not found on the system! Make sure to build and/or install libotic first")
#endif()

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

include_directories(
        ${PHP_INCLUDES}
        ${OTIC_PACK_FILES}
        ${OTIC_UNPACK_FILES}
        ${OTIC_CORE_FILES}
)

#  php -dextension=`pwd`/libotic_php.so -f test.php
#FILE(GLOB KernelHeaders ./kernel/*.h)
#FILE(GLOB KernelSources ./kernel/*.c)

set(PHPOTIC_SRC_FILES
        ${PROJECT_SOURCE_DIR}/ext/hello/hello.c
        ${PROJECT_SOURCE_DIR}/ext/hello/php_hello.h
        ${KernelHeaders}
        ${KernelSources}
        )

#if (UNIX)
#    add_definitions(-DPHP_ATOM_INC -fvisibility=hidden)
#endif()

add_php_executable(demo/pack/simple_pack.php)
add_php_executable(demo/unpack/simple_unpack.php)
add_php_executable(demo/tsvParser/tsvParser.php)
add_php_executable(demo/pack/test.php)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    add_definitions(-DHAVE_CONFIG_H)
    set(SOURCE_FILES "${SOURCE_FILES};config.h")
endif()
include_directories(${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../include ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../src)
#add_library(php_test SHARED ${PHPOTIC_SRC_FILES})
add_library(otic_php SHARED src/otic_php.h src/otic_php.c)
#target_include_directories(otic_php PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(otic_php otic zstd m pthread)

install(
        TARGETS otic_php
        RUNTIME DESTINATION ${PHP_EXTENSION_DIR}
        LIBRARY DESTINATION ${PHP_EXTENSION_DIR}
        ARCHIVE DESTINATION ${PHP_EXTENSION_DIR}
        )
install(
        SCRIPT "${PROJECT_SOURCE_DIR}/cmake/installExt.cmake"
        )