set(OTIC_CORE_FILES
        ${PROJECT_SOURCE_DIR}/include/core/core.h
        ${PROJECT_SOURCE_DIR}/include/core/config.h
        ${PROJECT_SOURCE_DIR}/include/core/pack.h
        ${PROJECT_SOURCE_DIR}/include/core/unpack.h
        ${PROJECT_SOURCE_DIR}/src/core/core.c
        ${PROJECT_SOURCE_DIR}/src/core/pack.c
        ${PROJECT_SOURCE_DIR}/src/core/unpack.c
        )

add_library(otic_shared SHARED ${OTIC_CORE_FILES})
add_library(otic_static STATIC ${OTIC_CORE_FILES})
target_include_directories(otic_shared PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_include_directories(otic_static PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

set_target_properties(
        otic_shared
        PROPERTIES OUTPUT_NAME otic
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

set_target_properties(
        otic_static
        PROPERTIES OUTPUT_NAME otic
)

target_compile_options(otic_shared PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS otic_shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
install(TARGETS otic_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

set(OTIC_CORE_FILES ${OTIC_CORE_FILES} PARENT_SCOPE)