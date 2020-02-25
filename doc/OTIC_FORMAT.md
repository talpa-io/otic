## OTIC Format internals  

### Theory of Operation  
OTIC is a serialization library targeted toward compression of sensor data.  
The compression is done 2 folds. The first happens not only by encoding primitive types into a buffer and replacing already encountered sensors names by an unique ID 
but also avoiding identical values repetition. The second compression uses zstd to compress the buffer into the output stream 
after a specified threshold is reached.

### Header section  

A valid otic file starts with a header frame (6 Bytes):  
  
| magic | features | version |  
| --- | --- | --- |  
|4 bytes | 1 byte | 1 byte |  

- 4 Bytes: The Magic sequence is always `0x79 0xa9 0x46 0x35`  
Except for the `0x79` byte, the rest of the magic sequence was generated from the following code:  
```python
import random
print(*map(lambda x: hex(int(random.random() * 10000) % int(random.random() * 1000) % 255), range(3)))
``` 
They were chosen so to be as random and therefore (most probably) as distinct from other files' magic as possible.
- 1 Byte: Features (reserved): `0x00`
- 1 Byte: Format version: Generated from the CMake Variable: `OTIC_VERSION_MAJOR` 

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

### Meta Data 
| meta tag | channelId | [content] |  
| --- | --- | --- |
| 1 byte | 1 byte | (variadic) | 
Metadata describe how the file is to be handled. They inform the decompressor about the current state of the compressor.  
Current Meta tag are set from the current enum:  
```c
typedef enum
{
    OTIC_META_TYPE_CHANNEL_DEFINE,
    OTIC_META_TYPE_CHANNEL_TYPE,
    OTIC_META_TYPE_COMPRESSION_METHOD,
    OTIC_META_TYPE_CHUNK_SIZE,
    OTIC_META_TYPE_DATA
} otic_meta_type_e;
```  
For example, the compressor specifies it's current buffer size with the Meta Tag: `OTIC_META_TYPE_CHUNK_SIZE`, followed 
by a byte specifying the channelID and a 4 bytes long size of its buffer. While reading the compressed file, the decompressor 
resizes it's bufferSize accordingly to avoid buffer overflows.  


### Data Frame  
| meta flag | channelId | chunk size |
| --- | --- | --- |
| 1 byte | 1 byte | 4 bytes |  

A Data Frame start with the meta tag `OTIC_META_TYPE_DATA`. The decompressor then selects the correct channel from the given 
`channelId` and reads a chunk of size `chunk size` from the input stream into the channel's buffer, which in turn gets 
decompressed using zstd . The resulting buffer gets decoded and the result flushed using the given callback.