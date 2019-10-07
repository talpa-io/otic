message(STATUS "Hey from PHP bindings")

cmake_minimum_required(VERSION 3.10)
project(php_otic C)

set(CMAKE_C_STANDARD 99)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0 -Wall -std=gnu99 -fvisibility=hidden")
set(CMAKE_C_FLAGS "-g -O2 -DHAVE_CONFIG_H")

function(FROM_SHELL VARIABLE)
    set(TO_RETURN "")
    execute_process(
            COMMAND ${ARGN}
            OUTPUT_VARIABLE TO_RETURN
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Output: ${TO_RETURN}")
    set(${VARIABLE} ${TO_RETURN} PARENT_SCOPE)
endfunction()

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
    message(STATUS ${X})
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


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
    add_definitions(-DHAVE_CONFIG_H)
    set(SOURCE_FILES "${SOURCE_FILES};config.h")
endif()

#add_library(php_test SHARED ${PHPOTIC_SRC_FILES})
add_library(otic_php SHARED src/otic_php.h src/otic_php.c)
#target_include_directories(otic_php PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(otic_php otic zstd m pthread)