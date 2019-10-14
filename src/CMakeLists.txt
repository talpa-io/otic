set(OTIC_CORE_FILES
        core/core.h
        core/core.c
        )

set(OTIC_PACK_FILES
        pack/pack.c
        pack/pack.h
        )

set(OTIC_UNPACK_FILES
        unpack/unpack.c
        unpack/unpack.h
        )

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
add_library(otic SHARED ${OTIC_PACK_FILES} ${OTIC_UNPACK_FILES} ${OTIC_CORE_FILES})