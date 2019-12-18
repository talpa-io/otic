#include <stdint.h>
#include <stddef.h>
#include "../../include/utility/format.h"

format_t format;

uint8_t format_fuzz(char* line)
{
    format_parse(&format, line);
    return 0;
}

int LLVMFuzzerTestOneInput(char* line)
{
    format_init(&format, '\t', 5);
    format_fuzz(line);
    format_close(&format);
    return 0;
}
