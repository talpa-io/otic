<?php

declare(strict_types=1);

namespace Otic;
use PHPUnit\Framework\TestCase;
use Otic\OticException;

class LibOticExceptionTest extends TestCase
{
    static private $errorCodes = [
        LibOticException::NONE => "None",
        LibOticException::INVALID_POINTER => "Invalid Pointer",
        LibOticException::BUFFER_OVERFLOW => "Buffer Overflow",
        LibOticException::INVALID_TIMESTAMP => "Invalid Timestamp",
        LibOticException::ENTRY_INSERTION_FAILURE => "Entry insertion failed",
        LibOticException::ZSTD => "zstd operation failed",
        LibOticException::FLUSH_FAILED => "Flush failure",
        LibOticException::EOF => "Invalid End Of File",
        LibOticException::INVALID_FILE => "Invalid file",
        LibOticException::DATA_CORRUPTED => "Data corrupted",
        LibOticException::VERSION_UNSUPPORTED => "Unsupported Version detected",
        LibOticException::ROW_COUNT_MISMATCH => "Rows count mismatch",
        LibOticException::INVALID_ARGUMENT => "Invalid Argument",
        LibOticException::AT_INVALID_STATE => "Operation at invalid state",
        LibOticException::ALLOCATION_FAILURE => "Allocation failure"
    ];

    public function testErrors()
    {
        foreach ($this::$errorCodes as $errorNum => $errorMessage)
        {
            try {
                throw new LibOticException($errorNum);
            } catch (LibOticException $e) {
                static::assertSame($errorMessage, $e->getMessage());
            }
        }
        try {
            throw new LibOticException(124);
        } catch (LibOticException $e) {
            static::assertSame("Unknown", $e->getMessage());
        }
    }
}
