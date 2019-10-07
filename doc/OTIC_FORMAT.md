# OTIC Format internals



## Otic Sections

### Header section

A valid otic file starts with a header frame (6 Bytes): 

- 4 Bytes: The Magic sequence is always `0x79 0x67 0x07 0xff`
- 1 Byte: Features (reserved): `0x00`
- 1 Byte: Format version: `0x01` 

```cpp
typedef struct
{
    uint8_t magic[OTIC_MAGIC_SIZE];
    uint8_t features;
    uint8_t version;
} __attribute__((packed)) otic_header_t;
```

### Meta Section

The Meta sections starts immediately after the Header section with at least a `otic_meta_channel_t` with 
frame_type `OTIC_END_OF_META`:

```
typedef struct
{
    otic_meta_frame_t frame_type;
    uint8_t ch_id;
    uint16_t opt_value;
} otic_meta_channel_t
```


```
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
$writer = new OticWriter();
$writer->setOutput(file1.otic);

$ch1 = $writer->defineRawChannel(1, COMPRESSION_NONE);
$ch2 = $writer->defineBasicSensorChannel(2, COMPRESSION_ZSTD);

$ch2->setMaxBuffer(64k)

$ch1->inject(time(), file_get_contents("/dev/webcam1");
$ch2->inject(time(), "sensor1", 20ihjfs, "km/h");
$ch1->inject(time(), file_get_contents("/dev/webcam1");
$ch2->inject(time(), "sensor1", 20ihjfs, "km/h");
$ch2->inject(time(), "sensor2", 20ihjfs, "km/h");

$writer->close();

```








