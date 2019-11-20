## Installation  

The whole project is built using Cmake.  

## Extension Structure:  
```php
<? namespace Otic;
class OticPack
{
    private $flusher = null;
    public function __construct(callable $flusher);
    public function __toString(): string;
    public function __debugInfo(): array;
    public function defineChannel(int $channelId, int $channelType): OticPackChannel;
    public function closeChannel(int $channelId): bool;
    public function flush();
    public function __destruct();
}

class OticPackChannel
{
    public function __construct();
    public function __toString(): string;
    public function __debugInfo(): array;
    public function inject(float $timestamp, string $sensorName, string $sensorUnit, $value): bool;
    public function flush();
    public function __destruct();
}

class OticPackStream
{
    public function __construct(resource fileOut);
    public function __toString(): string;
    public function __debugInfo(): array;
    public function defineChannel(int $channelId, int $channelType): OticPackChannel;
    public function closeChannel(int $channelId): bool;
    public function flush();
    public function __destruct();
}  

class OticUnpack
{
    public function __construct(callable $fetcher, callable $seeker = null);
    public function __toString(): string;
    public function __debugInfo(): array;
    public function defineChannel(int $channelId, callable $flusher): bool;
    public function closeChannel(int $channelId): bool;
    public parse(): bool;
    public function __destruct();
}

class OticUnpackStream
{
    public function __construct(resource fileIn);
    public function __toString(): string;
    public function __debugInfo(): array;
    public function defineChannel(int $channelId, callable $flusher): bool;
    public function closeChannel(int $channelId): bool;
    public function parse(): bool;
    public function __destruct();
}
```