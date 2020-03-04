enable_testing()
function(add_test_executable testName)
    add_executable(
            ${testName}
            ${unity_SOURCE_DIR}/src/unity.h
            ${unity_SOURCE_DIR}/src/unity.c
            include/otic_test.h
            include/otic_test.c
            ${OTIC_CORE_FILES}
            ${ARGN}
    )
    add_test(NAME ${testName} COMMAND ${testName})
    target_link_libraries(${testName} unity zstd m)
    set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE C COMPILE_FLAGS "-DUNITY_INCLUDE_DOUBLE -DUNITY_DOUBLE_PRECISION=1e-12f")
    target_include_directories(${testName} PUBLIC ${PROJECT_SOURCE_DIR}/include/ include)
endfunction()

add_test_executable(test_test src/test_test.c)
add_test_executable(core_test src/base.c)
add_test_executable(format_test src/format.c ../../src/utility/format.c)
add_test_executable(pack_test src/pack.c)
#add_test_executable(aggreg_test src/aggregator.c ../../src/utility/aggregator.c ../../include/utility/aggregator.h)
