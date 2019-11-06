## Installation  

The whole project is built using Cmake.  

## Extension Structure:  
```php
<? namespace Otic;
class OticPack
{
    private $dumpFunction = null;
    public function __construct(callable $dump);
    public function __toString(): string;
    public function __debugInfo(): string;
    public function defineChannel(int $channelId, int $channelType): bool;
    public function closeChannel(int $channelId): bool;
    public function close();
    public function flush();
    public function __destruct();
}

class OticPackChannel
{
    public function __construct();
    public function inject(float $timestamp, string $sensorName, string $sensorUnit, $value): bool;
    public function close();
    public function flush();
    public function __destruct();
}

class OticPackStream
{
    public function __construct(string filePath);
    public function __destruct();
}

```