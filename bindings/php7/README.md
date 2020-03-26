## Overview  
The current directory contains PHP bindings of OTIC.  
This binding is written as a PHP extension using the Zend Engine version 7 and therefore works for early version of PHP7 
(tested on PHP 7.3. It seems some Zend functions are going to change in the next versions: i.e. `zend_call_method_with_0_params` 
will require `zend_object *`  instead of `zval*` in the first argument).  

## Installation  
The Extension is to be built using cmake.  
 - **Build**:  
```bash
mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make
```
This will create an `libotic_php.so` in the `lib` directory.  
 - **Install**
```bash
sudo make install
```
still in the `build` directory that was created a step above. The previous command will copy `libotic_php.so` into the 
corresponding PHP extension directory and create a `libotic_php.ini` file with content `extension=libotic_php` inside the 
 PHP extension config directory. 
To generate the stub file. Run (still in the build directory):  
```bash
cmake --build . --target generate_from_stub
``` 
This will create an `otic.php` file from the stub `otic.php.stub` file.  

## Uninstall  

```bash
sudo make uninstall
```
or 
```bash
sudo cmake --build . --target uninstall
```
 
## Extension Structure  
```php
<?php namespace Otic;

/**
*   Not inside the namespace: \a Otic
*/
function getLibOticVersion() : string
{
}

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
    public function __construct(int $errorNo)
    {
    }
}

class OticException extends Exception
{
}

class OticPackChannel
{
    const TYPE_SENSOR = 0x00;
    const TYPE_BINARY = 0x01;
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
    public function resizeBucket(int $bufferSize) : void
    {
    }
    public function flush()
    {
    }
    public function close()
    {
    }
}

class OticPack
{
    public function __construct($fileHandle)
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
    public function flush()
    {
    }
    public function closeChannel(int $channelId)
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
    public function __construct($fileHandle)
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

```
