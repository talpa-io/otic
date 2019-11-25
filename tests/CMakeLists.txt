enable_testing()
function(OTIC_ADDTEST testName)
    add_executable(
            ${testName}
            ${ARGN}
            test_asserts.h
            ${PROJECT_SOURCE_DIR}/src/core/core.h
            ${PROJECT_SOURCE_DIR}/src/core/core.c
    )
    target_link_libraries(${testName} zstd m pthread)
    add_test(NAME ${testName} COMMAND ${testName})
    set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(${testName} PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
endfunction()

OTIC_ADDTEST(core_test base/base.c)
OTIC_ADDTEST(pack_test pack/pack.c)
OTIC_ADDTEST(pack_unpack_test
        pack_unpack/pack_unpack.c
        ../include/errHand/errHand.c
        ../include/errHand/errHand.h
        )
OTIC_ADDTEST(format_test
        format/format.c
        ../include/format/format.h
        ../include/format/format.c
        )

OTIC_ADDTEST(packChannel_test
        packChannel/packChannel.c
        ../include/errHand/errHand.c
        ../include/errHand/errHand.h
        )