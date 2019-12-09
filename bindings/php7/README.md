## Extension Structure  
```php
class OticException extends Exception
{
}

class LibOticException extends Exceptions 
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
    
    public function __construct(int $errorCode);
}

```


## Changes

- Use `\Otic` - Namespace 
- Provide / Maintain Class Reference in otic-php.php
- Provide Statistics for Reader and Writer
    - Which datatypes were written / read

- Provide `\Otic\Otic` Class with Constants `CHANNEL_TYPE_SENSOR`, `CHANNEL_TYPE_BINARY`
- Investigate Reader Segfault
- Support Callable as callback (Speed comparison / Lambda / Function call)

