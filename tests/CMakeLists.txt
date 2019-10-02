message(STATUS "Hallo World from OTIC Tests")
enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/include)

function(OTIC_ADD_TEST testName)
    add_executable(${testName} ${ARGN})
    add_test(${testName} testName)
endfunction()

OTIC_ADD_TEST(format_test
        format/format_test.c
        ../include/format/format.c
        ../include/format/format.h
        )
