<?php

namespace Otic;

use phpDocumentor\Reflection\Types\Resource_;
use PHPUnit\Framework\Constraint\IsIdentical;
use PHPUnit\Framework\ExpectationFailedException;
use PHPUnit\Framework\TestCase;
use Otic\OticException;

class OticUnpackTest extends TestCase
{
    private $unPacker = null;
    private $inputFile = null;

    const INPUT_FILE_NAME = "inFile.otic";
    const OUTPUT_FILE_NAME = "outFile.otic";

    private function normalizePath(string $path) : string
    {
        return dirname(__FILE__)."/$path";
    }

    private function generateOticFile()
    {
        $this->inputFile = fopen($this->normalizePath(self::INPUT_FILE_NAME), "w");
        $packer = new OticPack($this->inputFile);
        $channel = $packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $channel->inject(1234, "sensorName1", "sensorUnit", 2323);
        $packer->close();
        fclose($this->inputFile);
    }

    private function destroyOticFile()
    {
        unlink($this->normalizePath(self::INPUT_FILE_NAME));
    }

    protected function setUp()
    {
        parent::setUp();
        $this->generateOticFile();
        $this->inputFile = fopen($this->normalizePath(self::INPUT_FILE_NAME), "r");
        $this->unPacker = new OticUnpack($this->inputFile);
    }

    protected function tearDown()
    {
        parent::tearDown();
        $this->unPacker->close();
        fclose($this->inputFile);
    }

    public function test__toString()
    {
        static::assertIsString($this->unPacker->__toString());
    }

    public function testSelectChannel()
    {
        $channel = $this->unPacker->selectChannel(0x01, function ($ts, $sn, $su, $v) {
            echo "$ts\t$sn\t$su\t$v\n";
        });
        while (!feof($this->inputFile))
            $this->unPacker->parse();
        $channel->close();
        static::assertIsString($this->unPacker->__toString());
    }

    public function testMultipleChannels()
    {
        $channel1 = $this->unPacker->selectChannel(0x01, function () { });
        $channel2 = null;
        try {
            $channel2 = $this->unPacker->selectChannel(0x01, function (){});
        } catch (LibOticException $e) {
            static::assertSame("Invalid Argument", $e->getMessage());
        } finally {
            static::assertNull($channel2);
        }
        $channel1->close();
        $this->unPacker->clearErrorFlag();
        $channel2 = $this->unPacker->selectChannel(0x01, function () {});
        static::assertIsObject($channel2);
        $channel3 = $this->unPacker->selectChannel(0x02, function () {});
        static::assertIsObject($channel3);
        $channel4 = $this->unPacker->selectChannel(0x04, function () {});
        static::assertIsObject($channel3);
    }

    public function testInvalidChannelId()
    {
        $channel1 = null;
        try {
            $channel1 = $this->unPacker->selectChannel(-1, function() {});
        } catch (\Otic\OticException $e) {
            static::assertSame("Invalid Channel ID! Reason: Valid Range (int): 0 - 255", $e->getMessage());
        } finally {
            static::assertNull($channel1);
        }
        $this->unPacker->clearErrorFlag();
        try {
            $channel1 = $this->unPacker->selectChannel(0x100, function (){});
        } catch (\Otic\OticException $e) {
            static::assertSame("Invalid Channel ID! Reason: Valid Range (int): 0 - 255", $e->getMessage());
        } finally {
            static::assertNull($channel1);
        }
    }
}
