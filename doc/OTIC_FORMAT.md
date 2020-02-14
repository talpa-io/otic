# OTIC Format internals



## Otic Sections

### Header section

A valid otic file starts with a header frame (6 Bytes): 

- 4 Bytes: The Magic sequence is always `0x79 0xa9 0x46 0x35`  
Except for the `0x79` byte, the rest of the magic sequence was generated from the following code:  
```python
import random
print(*map(lambda x: hex(int(random.random() * 10000) % int(random.random() * 1000) % 255), range(3)))
``` 
They were chosen so to be as random and therefore (most probably) as distinct from other files' magic as possible.
- 1 Byte: Features (reserved): `0x00`
- 1 Byte: Format version: `0x01` 

```c
typedef struct
{
    uint8_t magic[OTIC_MAGIC_SIZE];
    uint8_t features;
    uint8_t version;
} PACK_MAX otic_header_t;
```  
where  
```c
#if OTIC_WIN32
#define MAX_PACK
#elif defined(__GNUC__) && __GNUC__ >= 4
#define MAX_PACK __attribute__((packed))
#else
#define MAX_PACK
#endif
```

### Meta Section  
Meta tags describe how the decompressor should behave (kind of like commands).
The Meta sections starts immediately after the Header section with at least a `otic_meta_channel_t` with 
frame_type `OTIC_END_OF_META`:

```c
typedef struct
{
    otic_meta_frame_t frame_type;
    uint8_t ch_id;
    uint16_t opt_value;
} otic_meta_channel_t
```


```c
typedef enum
{
    OTIC_CHANNEL_DEFINE=0x01
    /* opt_value: 0x00 => No compression; 0x01 => ZStd; 0x02 => Zlib; 0x03 => deflate */
    
    OTIC_CHANNEL_TYPE_SET=0x02
    /* opt_value: 0x00 => Raw Data ; 0x01 => Basic Sensor Data */
    
    OTIC_END_OF_META=0xff
} otic_meta_frame_t
```




```
typedef enum
{
    OTIC_META_CHANNEL_FLAG_COMPRESSION_ZLIB,
    OTIC_META_CHANNEL_FLAG_COMPRESSION_ZLIB
}
```

### Payload Section 

Each Payload section start with a Channel frame


```
typedef struct
{
    uint8_t ch_id;
    uint32_t data_len;
    uint64_t start_timestamp; /* full seconds */
} otic_payload_t
```
Followed by `data_len` bytes of payload. (Compressed with the methods defined in
Meta Header of the channel)

Channel id `0x00` is reserved for `END_OF_FILE` flag.




## Demo


```
class FileOutputDriver implements OticStream {

    private $fd;

    public function __construct($filename) 
    {
        $this->fd = fopen($filename, "r");
    } 


    /**
     * Returns binary data or null if end of file is reached
     */
    public function read(int $lenght) : string 
    {
        if (feof ($this->fd))
            return null;
        return fread($this->$fd, $length)
    }


}


$outputDriver = new FileOutputDriver()

$writer = new OticWriter();
$writer->setOutput(file1.otic);

$ch1 = $writer->defineRawChannel(1, COMPRESSION_NONE);
$ch2 = $writer->defineBasicSensorChannel(2, COMPRESSION_ZSTD);


$ch1->inject(time(), file_get_contents("/dev/webcam1");
$ch2->inject(time(), "sensor1", 20ihjfs, "km/h");
$ch1->inject(time(), file_get_contents("/dev/webcam1");
$ch2->inject(time(), "sensor1", 20ihjfs, "km/h");
$ch2->inject(time(), "sensor2", 20ihjfs, "km/h");

$writer->close();

```








