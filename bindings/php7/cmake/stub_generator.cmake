function(GENERATE_FROM_STUB INPUT_PATH OUTPUT_PATH)
    add_custom_target(generate_from_stub
            COMMAND cp -v ${INPUT_PATH} ${OUTPUT_PATH}
            )
endfunction()