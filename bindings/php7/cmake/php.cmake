# Build this target! Don't run it else your IDE will try to `./` the output binary.
function(add_php_executable EXECUTABLE_PATH)
    string(REPLACE / _ STRIPPED_NAME ${EXECUTABLE_PATH})
    add_custom_target(${STRIPPED_NAME}
#            COMMAND USE_ZEND_ALLOC=0 valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes php -dextension=${PROJECT_SOURCE_DIR}/lib/libotic_php.so -f ${PROJECT_SOURCE_DIR}/${EXECUTABLE_PATH}
            COMMAND php -dextension=${PROJECT_SOURCE_DIR}/lib/libotic_php.so -f ${PROJECT_SOURCE_DIR}/${EXECUTABLE_PATH}
            USES_TERMINAL
            )
endfunction()