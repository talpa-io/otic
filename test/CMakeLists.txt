enable_testing()
function(OTIC_ADDTEST testName)
    add_executable(
            ${testName}
            ${ARGN}
            test_asserts.h
            ../include/core/base.h
            ../src/core/base.c
    )
    target_link_libraries(${testName} zstd m pthread)
    add_test(NAME ${testName} COMMAND ${testName})
    set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE C)
    target_include_directories(${testName} PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
endfunction()

OTIC_ADDTEST(core_test ctest/base.c)
OTIC_ADDTEST(pack_test ctest/pack.c)
OTIC_ADDTEST(pack_unpack_test
        ctest/pack_unpack.c
        ../src/utility/errHand.c
        ../include/utility/errHand.h
        )
OTIC_ADDTEST(format_test
        ctest/format.c
        ../include/utility/format.h
        ../src/utility/format.c
        )

OTIC_ADDTEST(packChannel_test
        ctest/packChannel.c
        ../src/utility/errHand.c
        ../include/utility/errHand.h
        )