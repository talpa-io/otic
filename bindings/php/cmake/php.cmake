# Build this target! Don't run it else CMake will try to `./` the output binary.

function(add_php_executable EXECUTABLE_PATH)
    string(REPLACE / _ STRIPPED_NAME ${EXECUTABLE_PATH})
    add_custom_target(${STRIPPED_NAME}
            COMMAND php -dextension=${PROJECT_SOURCE_DIR}/lib/libotic_php.so -f ${PROJECT_SOURCE_DIR}/${EXECUTABLE_PATH}
            USES_TERMINAL
            )
endfunction()