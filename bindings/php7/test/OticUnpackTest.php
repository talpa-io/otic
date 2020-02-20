<?php

namespace Otic;

use phpDocumentor\Reflection\Types\Resource_;
use PHPUnit\Framework\Constraint\IsIdentical;
use PHPUnit\Framework\TestCase;


trait PackSingleton
{

}


class OticUnpackTest extends TestCase
{
    private $unPacker = null;
    private $inputFile = null;
    private $outputFile = null;

    const INPUT_FILE_NAME = "dump/inFile.otic";
    const OUTPUT_FILE_NAME = "dump/outFile.otic";

    use PackSingleton;

    private function generateOticFile()
    {
        $this->inputFile = fopen(self::INPUT_FILE_NAME, "w");
        $packer = new OticPack($this->inputFile);
        $channel = $packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $channel->inject(1234, "sensorName1", "sensorUnit", 2323);
        $packer->close();
        fclose($this->inputFile);
    }

    private function destroyOticFile()
    {
        unlink(self::INPUT_FILE_NAME);
    }

    protected function setUp()
    {
        parent::setUp();
        $this->generateOticFile();
        $this->inputFile = fopen(self::INPUT_FILE_NAME, "r");
        $this->unPacker = new OticUnpack($this->inputFile);
    }

    protected function tearDown()
    {
        parent::tearDown();
        $this->unPacker->close();
        fclose($this->inputFile);
    }

//    public function testParse()
//    {
//
//    }

    public function test__toString()
    {
        static::assertIsString($this->unPacker->__toString());
    }

    public function testSelectChannel()
    {
        $channel = $this->unPacker->selectChannel(0x01, function ($ts, $sn, $su, $v){
            echo "$ts\t$sn\t$su\t$v\n";
        });
        while (!feof($this->inputFile))
            $this->unPacker->parse();
        static::assertIsString($this->unPacker->__toString());
    }

    public function testMultipleChannels()
    {
//        $channel1 = $this->unPacker->selectChannel(0x01, function ($ts, $sn, $su, $v) {
//            echo "$ts, $sn, $su, $v\n";
//        });

//        $channel2 = $this->unPacker->selectChannel(0x02, function ($ts, $sn, $su, $v) {
//            echo "$ts, $sn, $su, $v\n";
//        });
    }


//    public function test__construct()
//    {
//        echo $this->unPacker;
//
//    }

//    public function __destruct()
//    {
//        if ($this->inputFile !== null) {
//            $this->destroyOticFile();
//        }
//    }
}
