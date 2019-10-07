# Otic Sensor base Channel format


```
typedef struct
{
    uint8_t frame_type;
    uint16_t sensor_id;
} otic_payload_t

```


```
frame_type:

    OTIC_TYPE_NAME_ASSIGN - (followed by int8_t name_len: `sensorname:mu`)
    OTIC_TS_SHIFT - (followed by leb128)
    

```