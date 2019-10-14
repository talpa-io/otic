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
            return "None";
        case OTIC_ERROR_INVALID_POINTER:
            return "Invalid Pointer";
        case OTIC_ERROR_BUFFER_OVERFLOW:
            return "Buffer Overflow";
        case OTIC_ERROR_INVALID_TIMESTAMP:
            return "Invalid Timestamp";
        case OTIC_ERROR_ENTRY_INSERTION_FAILURE:
            return "Entry insertion failed";
        case OTIC_ERROR_ZSTD:
            return "zstd operation failed";
        case OTIC_ERROR_FLUSH_FAILED:
            return "Flush failure";
        case OTIC_ERROR_EOF:
            return "Invalid End Of File";
        case OTIC_ERROR_INVALID_FILE:
            return "Invalid file";
        case OTIC_ERROR_DATA_CORRUPTED:
            return "Data corrupted";
        case OTIC_ERROR_VERSION_UNSUPPORTED:
            return "Unsupported Version detected";
        case OTIC_ERROR_ROW_COUNT_MISMATCH:
            return "Rows count mismatch";
        case OTIC_ERROR_INVALID_ARGUMENT:
            return "Invalid Argument";
        case OTIC_ERROR_AT_INVALID_STATE:
            return "Operation at invalid state";
        case OTIC_ERROR_ALLOCATION_FAILURE:
            return "Allocation failure";
        default:
            return "Unknown";
    }
}

void printOticError(otic_errors_e e)
{
    fprintf(stderr, "Error: %s\n", otic_getError(e));
}