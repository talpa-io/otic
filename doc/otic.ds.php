<?php
/**
 * This is only a DS file for code completion.
 *
 * Functionality is provided by libotic extension. Make sure
 * to have it loaded.
 *
 */
namespace Otic;

use Exception;

/**
*   Not inside the namespace: \a Otic
*/
function getLibOticVersion() : string
{
}

class LibOticException extends Exception
{
    const NONE = 0x00;
    const INVALID_POINTER = 0x01;
    const BUFFER_OVERFLOW = 0x02;
    const INVALID_TIMESTAMP = 0x03;
    const ENTRY_INSERTION_FAILURE = 0x04;
    const ZSTD = 0x05;
    const FLUSH_FAILED = 0x06;
    const EOF = 0x07;
    const INVALID_FILE = 0x08;
    const DATA_CORRUPTED = 0x09;
    const VERSION_UNSUPPORTED = 0x0A;
    const ROW_COUNT_MISMATCH = 0x0B;
    const INVALID_ARGUMENT = 0x0C;
    const AT_INVALID_STATE = 0x0D;
    const ALLOCATION_FAILURE = 0x0E;
    public function __construct(int $errorNo)
    {
    }
}

class OticException extends Exception
{
}

class OticPackChannel
{
    public function __construct()
    {
    }
    public function __toString() : string
    {
    }
    public function inject(float $timestamp, string $sensorName, string $sensorUnit, $value)
    {
    }
    public function getTimeInterval(): array
    {
    }
    public function getSensorsList() : array
    {
    }
    public function close()
    {
    }
}

class OticPack
{
    public function __construct($file)
    {
    }
    public function __toString() : string
    {
    }
    public function defineChannel(int $channelId, int $channelType, int $features) : OticPackChannel
    {
    }
    public function close()
    {
    }
    function __destruct()
    {
    }
}

class OticUnpackChannel
{
    public function __construct()
    {
    }
    public function __toString() : string
    {
    }
    public function setFetchList(string ... $values)
    {
    }
    public function getTimeInterval(): array
    {
    }
    public function getSensorsList() : array
    {
    }
    public function close()
    {
    }
    public function __destruct()
    {
    }
}

class OticUnpack
{
    public function __construct($file)
    {
    }
    public function __destruct()
    {
    }
    public function __toString() : string
    {
    }
    public function parse()
    {
    }
    public function selectChannel(int $channelId, callable $flusher): OticUnpackChannel
    {
    }
    public function close()
    {
    }
}
