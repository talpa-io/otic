include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

function(add_demo DEMO_NAME)
    add_executable(${DEMO_NAME} ${ARGN} ${PROJECT_SOURCE_DIR}/include/otic.h ${OTIC_CORE_FILES})
    target_link_libraries(${DEMO_NAME} zstd m pthread)
endfunction()

add_demo(pack_demo
        pack_demo/pack_demo.c
        ../include/utility/errHand.h
        ../src/utility/errHand.c
        )

add_demo(unpack_demo
        unpack_demo/unpack_demo.c
        ../include/utility/errHand.h
        ../src/utility/errHand.c
        )

add_demo(tsvParser
        tsvParser/tsvParser.c
        ../src/utility/errHand.c
        ../include/utility/errHand.h
        ../include/utility/format.h
        ../src/utility/format.c
        ../src/utility/compare.c
        ../include/utility/compare.h
        )

set_target_properties(
        tsvParser
        PROPERTIES OUTPUT_NAME otic
)

install(
        TARGETS tsvParser
        RUNTIME DESTINATION bin
        )

#target_compile_options(tsvParser PRIVATE -Wall -Wextra -pedantic -Werror)
#target_compile_options(tsvParser PRIVATE -Wall -Wextra -pedantic)
