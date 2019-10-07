# Raw format 


## Payload section

```
leb128 timestamp_shift;
int32_t raw_data_len; 
raw_data
```



## Example


Writer:
```
$ch1 = $writer->defineRawChannel(3);
$ch1->inject(time(), 

$ch2 = $writer->defineRawChannel(4);
```


Reader:

```
$reader->listChannels()

$ch1 = $reader->selectRawChannel(3);

$ch1->setOnData(function ($ts, $data) {
});

$ch2 = $reader->selectBasicSensor(1);
$ch2->setOnData(function ($ts, $sensorName, $measureUnit, $value) {
});

$reader->read();
```
