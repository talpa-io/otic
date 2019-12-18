function(add_fuzzing_executable TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/ ${PROJECT_SOURCE_DIR}/include)
    target_compile_options(${TARGET_NAME} PRIVATE $<$<C_COMPILER_ID:Clang>:-g -O1 -fsanitize=address,fuzzer>)
    target_link_libraries(${TARGET_NAME} PRIVATE $<$<C_COMPILER_ID:Clang>:-fsanitize=address,fuzzer> zstd m)
endfunction()

add_fuzzing_executable(format_fuzz ../../src/utility/format.c format_fuzz.c )