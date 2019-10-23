include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

function(add_demo DEMO_NAME)
    add_executable(${DEMO_NAME} ${ARGN})
    target_link_libraries(${DEMO_NAME} otic zstd m pthread)
endfunction()

add_demo(pack_demo
        pack_demo/pack_demo.c
        ../include/errHand/errHand.h
        ../include/errHand/errHand.c
        )

add_demo(unpack_demo
        unpack_demo/unpack_demo.c
        ../include/errHand/errHand.h
        ../include/errHand/errHand.c
        )

add_demo(tsvParser
        tsvParser/tsvParser.c
        ../include/errHand/errHand.c
        ../include/errHand/errHand.h
        ../include/format/format.h
        ../include/format/format.c
        )
