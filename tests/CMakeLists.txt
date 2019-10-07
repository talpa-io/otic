enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

function(OTIC_ADDTEST testName)
    add_executable(${testName} ${ARGN} test_asserts.h)
    target_link_libraries(${testName} otic zstd m pthread)
    add_test(${testName} testName)
    set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE C)
endfunction()

OTIC_ADDTEST(core_test base/base.c)
OTIC_ADDTEST(pack_test pack/pack.c)
OTIC_ADDTEST(
        pack_unpack_test
        pack_unpack/pack_unpack.c
        ../include/errHand/errHand.c
        ../include/errHand/errHand.h
    )
OTIC_ADDTEST(format_test
        format/format.c
        ../include/format/format.h
        ../include/format/format.c
        )