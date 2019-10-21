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

add_library(otic SHARED
        ${OTIC_PACK_FILES}
        ${OTIC_UNPACK_FILES}
        ${OTIC_CORE_FILES}
        )

target_include_directories(otic
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        )

set_target_properties(
        otic
        PROPERTIES OUTPUT_NAME otic
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
