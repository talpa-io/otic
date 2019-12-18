<?php
/**
 * Created by PhpStorm.
 * User: matthias
 * Date: 18.12.19
 * Time: 11:54
 */


interface OticInputHandler {
    public function read(int $length) : ?+string;
}

interface OticOutputHandler {
    public function write(string $data);
    public function close();
}




class OticFileInputHandler implements OticInputHandler {

    private $fd;

    public function __construct($filename)
    {
        $this->fd = fopen($filename, "r");
    }


    /**
     * Returns binary data or null if end of file is reached
     */
    public function read(int $length) : ?string
    {
        if (feof ($this->fd))
            return null;
        return fread($this->fd, $length);
    }
}

class OticFileOutputHandler implements OticOutputHandler {

    private $fd;

    public function __construct(string $filename)
    {
        $this->fd = fopen($filename, "w+");
    }


    public function write(string $data)
    {
        fwrite($this->fd, $data);
    }

    public function close()
    {
        fclose($this->fd);
    }
}





$writer = new OticWriter(new OticFileOutputHandler("/tmp/somefile"));

$ch1 = $writer->defineRawChannel(1, COMPRESSION_NONE);
$ch2 = $writer->defineBasicSensorChannel(2, OticWriter::COMPRESSION_ZSTD);


$ch1->inject(time(), file_get_contents("/dev/webcam1");
$ch2->inject(time(), "sensor1", 20ihjfs, "km/h");
$ch1->inject(time(), file_get_contents("/dev/webcam1");
$ch2->inject(time(), "sensor1", 20ihjfs, "km/h");
$ch2->inject(time(), "sensor2", 20ihjfs, "km/h");

// Will call Output Handler ->close() implicit
$writer->close();




$reader = new OticReader(new OticFileInputHandler("/tmp/somefile"));

$reader->getInfo();
// [0x1 => ["... matadata"]]



$channel1 = $reader->selectBasicSensorChannel(0x01, ["options" => []]);

// Optional: Get unaggregated data
$channel1->setFetchList([]); // Sensors to output

// Optional Aggregation:
$channel1->addAggregator($sensorName, $alias, $interval=1, OticAggregator::AGGREGATE_MIN);

// Aggregators: min,max,avg,sum,count,first
// Aggretation < 1sec ->
// interval: 0.5: 13.0, 13.5, 14.0
// interval: 0.3: 13.0, 13.3333, 13.6666, 14.0
// Achtung: float! 14.0000001 (nicht)

$channel1->setOnDataCallback(function ($ts, $alias, $mu, $value=null) {

});







$channel2 = $reader->selectRawChannel(0x02, []);
$channel2->setOnDataCallback(function ($ts, $rawData) {

});

$reader->parse();

$channel1->getFirstTs(); // Return null if no data in channel
$channel1->getLastTs();


$stats = $reader->getStats();
/**
 * [ 0x01 => [
 *      "int" => 289348,
 *      "float" => 92
 * ]
 */


$reader->close();