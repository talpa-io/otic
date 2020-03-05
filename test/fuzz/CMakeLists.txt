function(add_fuzzing_executable TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN} ${OTIC_CORE_FILES} ${OTIC_UTILITY_FILES})
    target_include_directories(
            ${TARGET_NAME}
            PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/include/core
            ${PROJECT_SOURCE_DIR}/include/utility
    )
    target_compile_options(${TARGET_NAME} PRIVATE $<$<C_COMPILER_ID:Clang>:-g -O1 -fsanitize=address,fuzzer>)
    target_link_libraries(${TARGET_NAME} PRIVATE $<$<C_COMPILER_ID:Clang>:-fsanitize=address,fuzzer> zstd m)
endfunction()

add_fuzzing_executable(format_fuzz ../../src/utility/format.c format_fuzz.c)
add_fuzzing_executable(packUnpack packUnpack.c)