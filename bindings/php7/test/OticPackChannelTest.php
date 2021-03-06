<?php

namespace Otic;
use phpDocumentor\Reflection\DocBlock\Tags\Param;
use phpDocumentor\Reflection\Types\Resource_;
use PHPUnit\Framework\Constraint\IsIdentical;
use PHPUnit\Framework\TestCase;

use Otic\OticPackChannel;
use Otic\OticPack;


trait RandomValGenerator
{
    public $startTs = 12345;
    private $row = [
        "timestamp" => 12345,
        "sensorName" => "",
        "sensorUnit" => "",
        "val" => ""
    ];

    public function generate(&$stats) : array
    {
        static $TOTAL_SENSORS = 20;

        $randomTs = static function(float $currentTs) use (&$stats) {
            $randVal = rand(1, 10000);
            if (($randVal % 2 != 0) && ($randVal % 3 != 0) ) {
                $currentTs += random_int(1, 1000) / random_int(4, 45);
                ++$stats["ts_shift"];
            } else {
                ++$stats["ts_unmodified"];
            }
            return $currentTs;
        };

        $randomSensor = static function (int $id) use ($TOTAL_SENSORS) : array {
            return ["sensor".$id, "unit".$id];
        };

        $randomFloat = static function () use (&$stats) : float  {
            ++$stats["float"];
            return (rand(0, 10000000) / rand(1, 3434123));
        };

        $randomInt = static function () use (&$stats): int {
            ++$stats["int"];
            return rand(0, 3434334543);
        };

        $randomString = static function(int $length = 1000) use (&$stats) : string
        {
            $randomModASCII = static function () {
                return random_int(32, 126);
            };
            $res = "";
            $length = random_int(0, $length);
            for ($counter = 0; $counter < $length; ++$counter)
                $res .= chr($randomModASCII());
            ++$stats["string"];
            return $res;
        };

        // TODO: Array, bool Randomization

        $randomVal = static function (int $id) use ($randomString, $randomInt, $randomFloat) {
            $valRandomizer = [$randomInt, $randomFloat, $randomString];
            return $valRandomizer[$id % 3]();
        };

        $id = random_int(0, $TOTAL_SENSORS);

        $this->row["timestamp"] = $randomTs($this->row["timestamp"]);
        $current = $randomSensor($id);
        $this->row["sensorName"] = $current[0];
        $this->row["sensorUnit"] = $current[1];
        $this->row["val"] = $randomVal($id);
        return $this->row;
    }

    public function reset()
    {
        $this->row["timestamp"] = $this->startTs;
    }
}

class OticPackChannelTest extends TestCase
{
    private $packer = null;
    private $outputFile = null;

    use RandomValGenerator;

    private static function assertAlmostEqual($expected, $actual, string $message = ""): void
    {
        $round = static function ($values) {
            $ret = [];
            foreach ($values as $value)
                $ret[] = ((int)$value * 10000) / 10000;
        };
        static::assertThat($round($actual), new IsIdentical($round($expected)), $message);
    }

    protected function setUp(): void
    {
        parent::setUp();
        $this->outputFile = fopen(__DIR__."/dump.otic", "w");
        $this->packer = new OticPack($this->outputFile);
    }

    protected function tearDown(): void
    {
        parent::tearDown();
        $this->packer->close();
        fclose($this->outputFile);
    }

    public function test__construct(): void
    {
        $channel = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        static::assertIsObject($channel);
        static::assertEmpty($channel->getSensorsList());
        static::assertEmpty($channel->getTimeInterval()[0]);
        static::assertEmpty($channel->getTimeInterval()[1]);
        foreach ($channel->getStats() as $key => $value)
            static::assertSame(0, $value);
        $channel->close();
    }

    public function testGetSensorsList(): void
    {
        $channel = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $temp = [];
        for ($value = 0; $value < 10; ++$value) {
            $temp[] = "sensor" . $value;
            $channel->inject($value, "sensor" . $value, "someUnit", $value);
        }
        static::assertSame($temp, $channel->getSensorsList());
        static::assertSame(count($temp), $channel->getStats()["typeColsAssigned"]);
        $channel->close();
    }

    public function testInject(): void
    {
        $channel = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $statsChecker = [
            "ts_shift" => 0,
            "ts_unmodified" => 0,
            "int" => 0,
            "float" => 0,
            "string" => 0
        ];
        $generated = [];
        for ($counter = 0; $counter < 10000; ++$counter)
        {
            $generated = $this->generate($statsChecker);
            $channel->inject($generated["timestamp"], $generated["sensorName"], $generated["sensorUnit"], $generated["val"]);
        }

        static::assertSame($statsChecker["string"], $channel->getStats()["typeString"]);
        static::assertSame($statsChecker["int"], $channel->getStats()["typeInteger"]);
        static::assertSame($statsChecker["float"], $channel->getStats()["typeDouble"]);
        static::assertSame(1, $channel->getStats()["typeTimestampSets"]);
        static::assertAlmostEqual([$this->startTs, $generated["timestamp"]], $channel->getTimeInterval());

        $channel->close();
    }

    public function testInvalidTs(): void
    {
        $channel = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $channel->inject(123, "abc", "cdef", 232);
        static::expectException(LibOticException::class);
        $channel->inject(122, "asd", "asdsad", "asd");
    }
}