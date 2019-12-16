enable_testing()
function(OTIC_ADDTEST testName)
    add_executable(
            ${testName}
            ${ARGN}
            ${unity_SOURCE_DIR}/src/unity.h
            ${unity_SOURCE_DIR}/src/unity.c
            ${OTIC_CORE_FILES}
    )
    add_test(NAME ${testName} COMMAND ${testName})
    target_link_libraries(${testName} unity zstd m)
    set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE C COMPILE_FLAGS "-DUNITY_INCLUDE_DOUBLE -DUNITY_DOUBLE_PRECISION=1e-12f")
    target_include_directories(${testName} PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
endfunction()

OTIC_ADDTEST(
        core_test
        ctest/base.c
)

OTIC_ADDTEST(
        format_test
        ctest/format.c
        ../src/utility/format.c
)

#OTIC_ADDTEST(pack_test ctest/pack.c)
#OTIC_ADDTEST(pack_unpack_test
#        ctest/pack_unpack.c
#        ../src/utility/errHand.c
#        ../include/utility/errHand.h
#        )
#OTIC_ADDTEST(format_test
#        ctest/format.c
#        ../include/utility/format.h
#        ../src/utility/format.c
#        )
#
#OTIC_ADDTEST(packChannel_test
#        ctest/packChannel.c
#        ../src/utility/errHand.c
#        ../include/utility/errHand.h
#        )