<?php namespace Otic;

use Exception;
/**
 * This function is not inside the 'php namespace' Otic
 * @return string The libOtic Version use during compilation
 * in the format {VERSION_MAJOR}.{VERSION_MINOR}.{VERSION_PATCH}
 */
function getLibOticVersion() : string
{
}


/**
 * Class LibOticException
 *
 * This class is used internally to throw Exceptions coming from the underlying C libotic core
 * @package Otic
 */
class LibOticException extends Exception
{
    const NONE                      = 0x00;
    const INVALID_POINTER           = 0x01;
    const BUFFER_OVERFLOW           = 0x02;
    const INVALID_TIMESTAMP         = 0x03;
    const ENTRY_INSERTION_FAILURE   = 0x04;
    const ZSTD                      = 0x05;
    const FLUSH_FAILED              = 0x06;
    const EOF                       = 0x07;
    const INVALID_FILE              = 0x08;
    const DATA_CORRUPTED            = 0x09;
    const VERSION_UNSUPPORTED       = 0x0A;
    const ROW_COUNT_MISMATCH        = 0x0B;
    const INVALID_ARGUMENT          = 0x0C;
    const AT_INVALID_STATE          = 0x0D;
    const ALLOCATION_FAILURE        = 0x0E;

    /**
     * LibOticException constructor.
     * @param int $errorNo The Error Number
     */
    public function __construct(int $errorNo)
    {
    }
}

/**
 * Class OticException
 *
 * Default exception thrown from libotic_php
 * @package Otic
 */
class OticException extends Exception
{
}

/**
 * Class OticPackChannel
 *
 * Object returned from defining a channel in the OticPack class.
 *
 * Note: It is not publicly constructable.
 * @package Otic
 */
class OticPackChannel
{
    const TYPE_SENSOR = 0x00;
    const TYPE_BINARY = 0x01;
    public function __construct()
    {
    }

    /**
     * Get a string representation of the state of the channel.
     *
     * This includes the current Error, State, Number of entries and the timeInterval
     *
     * @return string String representation of the channel.
     */
    public function __toString(): string
    {
    }

    /**
     * Inject, in other words insert an entry into a channel.
     * @param float $timestamp The current timestamp, preferably in seconds.
     * @param string $sensorName The name of the sensor.
     * @param string $sensorUnit The name of the unit.
     * @param mixed $value The value to insert.
     * @throws LibOticException In case of injection with a non-chronological timestamp, or when the length of the
     * <b>$sensorName</b> and <b>$sensorUnit</b> is larger than 255
     * @throws OticException In case of injection with an unsupported <b>$value</b> type.
     */
    public function inject(float $timestamp, string $sensorName, string $sensorUnit, $value): void
    {
    }

    /**
     * Returns the start timestamp and the end timestamp of the channel, according to the injected values.
     *
     * An array of null Values is returned when no entry was injected.
     * @return array [timestampBegin, timestampEnd]
     */
    public function getTimeInterval(): array
    {
    }

    /**
     * Returns a list of sensorNames as injected.
     * @return array list of sensor names.
     */
    public function getSensorsList(): array
    {
    }

    /**
     * Resize the underlying bucket size.
     * @throws LibOticException in case $newSize is smaller than the amount of cached values (flush() recommanded)
     * or when the $newSize is smaller than the minimal allow bucket size.
     * @param int $newSize The new Bucket Size.
     */
    public function resizeBucket(int $newSize): void
    {
    }

    /**
     * Return Statistics about the channels.
     *
     * The statistics may include: the number of entries of each supported types, the time interval and the total number
     * of entries.
     * @return array Statistics of the channel.
     */
    public function getStats(): array
    {
    }

    /**
     * Clear the errorFlag after a libOticException has been thrown.
     */
    public function clearErrorFlag(): void
    {
    }

    /**
     * Flush the bucket.
     */
    public function flush(): void
    {
    }

    /**
     * Close the channel.
     *
     * Note: Automatically called when the destructor of this class is called.
     */
    public function close(): void
    {
    }
}

/**
 * Class OticPack
 *
 * Defines an otic pack instance, which is the parent of pack channels.
 * @package Otic
 */
class OticPack
{
    /**
     * OticPack constructor.
     * Initialize underlying otic pack structures and stores the output stream.
     * @param resource $fileHandle Output file Handle.
     */
    public function __construct($fileHandle)
    {
    }

    /**
     * Represents the OticPack class.
     *
     * Representation may include the current ErrorCode, the State and the total number of channels.
     * @return string Representation of the Pack Object
     */
    public function __toString(): string
    {
    }

    /**
     * Create a new Pack Channel.
     * @param int $channelId The Identification of the current channel. Allowed are any integers in (0, 255].
     * @param int $channelType The type of the Channel. Allowed are OticPackChannel::TYPE_SENSOR and OticPackChannel::TYPE_BINARY.
     * @param int $features The features of the channel. Currently reserved.
     * @return OticPackChannel An instance of an <b>OticPackChannel</b>.
     * @throws OticException in case of invalid paramaters.
     * @throws LibOticException in case the channel already exists.
     */
    public function defineChannel(int $channelId, int $channelType, int $features): OticPackChannel
    {
    }

    /**
     * Close as specified channel using a <b>$channelId</b>.
     * @param int $channelId The valid channelId, as specified by the <b>defineChannel</b> method.
     * @throws LibOticException In case the channel was not found.
     */
    public function closeChannel(int $channelId): void
    {
    }

    /**
     * Flush every opened channel
     */
    public function flush(): void
    {
    }

    /**
     * Clear the Error Flag after an instance of <b>LibOticException</b> was thrown.
     */
    public function clearErrorFlag(): void
    {
    }

    /**
     * Close the <b>OticPack</b> instance and every channel it owns, and also flushes the bucket.
     *
     *Note: This function is called when the destructor is called. Many closing is therefore not needed.
     *@throws LibOticException In case the output stream was already closed.
     */
    public function close(): void
    {
    }

    /**
     * Flush and close every channel
     * @throws LibOticException In case the Output Stream is already closed.
     */
    function __destruct()
    {
    }
}

/**
 * Class OticUnpackChannel
 *
 * Provides an instance to access an channel returned by an <b>oticUnpack</b> instance.
 *
 * @package Otic
 */
class OticUnpackChannel
{
    /**
     * OticUnpackChannel constructor.
     *
     * Currently the default constructor
     */
    public function __construct()
    {
    }

    /**
     * Provide as string representation of the current channel.
     *
     * The representation may include the total entry read for each supported otic type, the total numbers of entries,
     * the total number of lines and the time interval.
     * @return string String Representation of the channel.
     */
    public function __toString(): string
    {
    }

    /**
     * Set a list of sensors to fetch when the parent Otic Unpack object parse the input file.
     * @param string $values At least one sensor name to fetch.
     */
    public function setFetchList(string ... $values): void
    {
    }

    /**
     * Provide the time interval of the parsed entries. Formatted as [timestamp_start, timestamp_end]
     * @return array Time interval of parsed entries.
     */
    public function getTimeInterval(): array
    {
    }

    /**
     * Provided the list of sensors parsed for the current channel by the parent <b>oticUnpack</b> instance
     * @return array The list of sensor names.
     */
    public function getSensorsList(): array
    {
    }

    /**
     *  Close the current channel.
     */
    public function close(): void
    {
    }

    /**
     * OticPackChannel destructor.
     *
     * Default destructor.
     */
    public function __destruct()
    {
    }
}

/**
 * Class OticUnpack
 *
 * Defines an otic pack instance, which is the parent of unpack channels.
 * @package Otic
 */
class OticUnpack
{
    /**
     * OticUnpack constructor.
     *
     * Initialize the underlying oticUnpack structures.
     * @param resource $fileHandle The Input Stream.
     */
    public function __construct($fileHandle)
    {
    }

    /**
     * Returns a string representation of the current unpack object.
     *
     * The representation may include the ErrorCode, the State and the total number of channels.
     * @return string string representation of the current object.
     */
    public function __toString(): string
    {
    }

    /**
     * Parse the input stream.
     * @throws LibOticException In case of invalid or unexpected behaviour.
     */
    public function parse(): void
    {
    }

    /**
     * Select a channel using the provided <b>$channelId</b> and provide a <b>$flusher</b> callback.
     * @param int $channelId The Identification of the channel, as provided in the Pack.
     * @param callable $flusher The callable to call, when data is found. Preferably of format function($timestamp, $sensorName, $sensorUnit, $value).
     * @return OticUnpackChannel A new instance of <b>OticUnpackChannel</b>.
     * @throws OticException In case the <b>$channelId</b> or <b>$flusher</b> is invalid.
     * @throws LibOticException In case the channel was already defined.
     */
    public function selectChannel(int $channelId, callable $flusher): OticUnpackChannel
    {
    }

    /**
     * Close the <b>OticUnpack</b> instance and every defined channel.
     * @throws LibOticException In case of unexpected behaviour.
     */
    public function close(): void
    {
    }

    /**
     * Calls the <b>close()</b> method and destroy the current object.
     */
    public function __destruct()
    {
    }
}