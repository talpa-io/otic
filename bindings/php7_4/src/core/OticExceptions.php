<?php
/**
 * Created by PhpStorm.
 * User: talpaadmin
 * Date: 04.12.19
 * Time: 15:06
 */

class OticException extends \Exception
{
    const OTIC_ERROR_NONE = 0;
    const OTIC_ERROR_INVALID_POINTER = 1;
    const OTIC_ERROR_BUFFER_OVERFLOW = 2;
    const OTIC_ERROR_INVALID_TIMESTAMP = 3;
    const OTIC_ERROR_ENTRY_INSERTION_FAILURE = 4;
    const OTIC_ERROR_ZSTD = 5;
    const OTIC_ERROR_FLUSH_FAILED = 6;
    const OTIC_ERROR_EOF = 7;
    const OTIC_ERROR_INVALID_FILE = 8;
    const OTIC_ERROR_DATA_CORRUPTED = 9;
    const OTIC_ERROR_VERSION_UNSUPPORTED = 10;
    const OTIC_ERROR_ROW_COUNT_MISMATCH = 11;
    const OTIC_ERROR_INVALID_ARGUMENT = 12;
    const OTIC_ERROR_AT_INVALID_STATE = 13;
    const OTIC_ERROR_ALLOCATION_FAILURE = 14;

    public function __construct($code = self::OTIC_ERROR_NONE, $message = "",  Throwable $previous = null)
    {
        parent::__construct(self::strError($code), $code, $previous);
    }

    public function __toString()
    {
        return __CLASS__.": [{$this->code}]: {$this->message}\n";
    }
    protected function strError($code)
    {
        switch ($code)
        {
            case self::OTIC_ERROR_NONE:
                return "None";
            case self:: OTIC_ERROR_INVALID_POINTER:
                return "Invalid Pointer";
            case self:: OTIC_ERROR_BUFFER_OVERFLOW:
                return "Buffer Overflow";
            case self:: OTIC_ERROR_INVALID_TIMESTAMP:
                return "Invalid Timestamp";
            case self:: OTIC_ERROR_ENTRY_INSERTION_FAILURE:
                return "Entry insertion failed";
            case self:: OTIC_ERROR_ZSTD:
                return "zstd operation failed";
            case self:: OTIC_ERROR_FLUSH_FAILED:
                return "Flush failure";
            case self:: OTIC_ERROR_EOF:
                return "Invalid End Of File";
            case self:: OTIC_ERROR_INVALID_FILE:
                return "Invalid file";
            case self:: OTIC_ERROR_DATA_CORRUPTED:
                return "Data corrupted";
            case self:: OTIC_ERROR_VERSION_UNSUPPORTED:
                return "Unsupported Version detected";
            case self:: OTIC_ERROR_ROW_COUNT_MISMATCH:
                return "Rows count mismatch";
            case self:: OTIC_ERROR_INVALID_ARGUMENT:
                return "Invalid Argument";
            case self:: OTIC_ERROR_AT_INVALID_STATE:
                return "Operation at invalid state";
            case self:: OTIC_ERROR_ALLOCATION_FAILURE:
                return "Allocation failure";
            default:
                return "Unknown";
        }
    }
}