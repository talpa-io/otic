<?php
namespace Otic;

use phpDocumentor\Reflection\Types\Resource_;
use PHPUnit\Framework\Constraint\IsIdentical;
use PHPUnit\Framework\TestCase;

class OticUnpackChannelTest extends TestCase
{
    private $packer = null;
    private $packChannel = null;
    const OUTPUT_FILE_NAME = "someOutputFile.otic";
    private $outputFile;
    private $injected = [];

    use RandomValGenerator; // See OticPackChannelTest;

    protected function setUp()
    {
        if (!extension_loaded("otic"))
            throw new \Exception("Otic could not be loaded");
        parent::setUp();
        $this->outputFile = fopen($this->normalizePath(self::OUTPUT_FILE_NAME), "w");
        $this->packer = new OticPack($this->outputFile);
        $this->packChannel = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x0);
    }

    protected function tearDown()
    {
        parent::tearDown();
        fclose($this->outputFile);
    }

    private function inject(float $ts, string $sn, string $su, $val)
    {
        $this->packChannel->inject($ts, $sn, $su, $val);
        $this->injected[] = "$ts\t$sn\t$su\t$val";
    }

    private function assertSameRow($row1, $row2)
    {
        $approx = function($ts) : int
        {
            return (int)($ts * 10000) / 10000;
        };
        static::assertSame(sizeof($row1), sizeof($row2));
        for ($counter = 0; $counter < sizeof($row1); ++$counter) {
            $res1 = explode("\t", $row1[$counter]);
            $res2 = explode("\t", $row2[$counter]);
            $res1[0] = $approx($res1[0]);
            $res2[0] = $approx($res2[0]);
            $res1[2] = $res2[2] = "";
            static::assertSame($res1, $res2, "Error at Row: $counter");
        }
    }

    private function normalizePath(string $path) : string
    {
        return dirname(__FILE__)."/$path";
    }

    public function test_correctRead()
    {
        $readStats = [
            "ts_shift" => 0,
            "ts_unmodified" => 0,
            "int" => 0,
            "float" => 0,
            "string" => 0
        ];
//        for ($counter = 0; $counter < 10000; ++$counter) {
//            $generated = $this->generate($readStats);
//            $this->inject($generated["timestamp"], $generated["sensorName"], $generated["sensorUnit"], $generated["val"]);
//        }

        $timestamp=1582612585.419277;
        for ($i=0; $i<86400; $i++) {
            $timestamp+=1;
            for ($i2=0; $i2<120; $i2++) {
                $unit = "u$i2";
                $name = "s$i2".$i%100;//bin2hex(random_bytes(rand(20,60)));
                $value = $i.$i2; //rand(0,999) . "." . rand(100000000000000,900000000000000);
                $this->inject($timestamp, $name, $unit, $value);
            }
        }

        $this->packer->close();
        fclose($this->outputFile);
        $this->outputFile = fopen($this->normalizePath(self::OUTPUT_FILE_NAME), "r");
        $unPacker = new OticUnpack($this->outputFile);
        $read = [];
        $unPackChannel = $unPacker->selectChannel(0x01, function ($ts, $sn, $su, $val) use (&$read) {
            $read[] = "$ts\t$sn\t$su\t$val";
        });
        while (!feof($this->outputFile))
            $unPacker->parse();
        static::assertSameRow($this->injected, $read);

        $unPacker->close();
    }

}

