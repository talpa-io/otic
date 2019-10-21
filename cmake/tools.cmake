message(STATUS "Loading: tools.cmake")

include(CheckIncludeFile)

macro(otic_fing_header HEADER_NAME OUTPUT_VAR)
    CHECK_INCLUDE_FILE(${HEADER_NAME} ${OUTPUT_VAR})
endmacro()

macro(get_search_dirs)
    set(search_dirs)
    execute_process(
            COMMAND cpp -print-search-dirs | tr : '\n'
            OUTPUT_VARIABLE search_dirs
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS ${search_dirs})
endmacro()

macro(copy_dir SRC_DIR DEST_DIR)
    file(COPY ${PROJECT_SOURCE_DIR}/SRC_DIR DESTINATION ${PROJECT_SOURCE_DIR}/DEST_DIR)
endmacro()