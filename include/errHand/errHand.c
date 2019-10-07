//
// Created by hp on 9/24/19.
//

#include <stdio.h>
#include "errHand.h"


static inline const char* otic_getError(otic_errors_e e)
{
    switch (e)
    {
        case OTIC_ERROR_NONE:
            return "Error: None";
        case OTIC_ERROR_INVALID_POINTER:
            return "Error: Invalid Pointer";
        case OTIC_ERROR_BUFFER_OVERFLOW:
            return "Error: Buffer Overflow";
        case OTIC_ERROR_INVALID_TIMESTAMP:
            return "Error: Invalid Timestamp";
        case OTIC_ERROR_ENTRY_INSERTION_FAILURE:
            return "Error: Entry insertion failed";
        case OTIC_ERROR_ZSTD:
            return "Error: Zstd Operation failed";
        case OTIC_ERROR_FLUSH_FAILED:
            return "Error: Flush failure";
        case OTIC_ERROR_EOF:
            return "Error: Invalid End Of File";
        case OTIC_ERROR_INVALID_FILE:
            return "Error: Invalid file";
        case OTIC_ERROR_DATA_CORRUPTED:
            return "Error: Data corrupted";
        case OTIC_ERROR_VERSION_UNSUPPORTED:
            return "Error: Unsupported Version detected";
        case OTIC_ERROR_ROW_COUNT_MISMATCH:
            return "Error: Rows count mismatch";
        default:
            return "Error: Unknown";
    }
}

void printOticError(otic_errors_e e)
{
    fprintf(stderr, "%s\n", otic_getError(e));
}