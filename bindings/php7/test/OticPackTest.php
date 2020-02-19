<?php

declare(strict_types=1);

namespace Otic;
use phpDocumentor\Reflection\Types\Resource_;
use PHPUnit\Framework\Constraint\IsIdentical;
use PHPUnit\Framework\TestCase;

class OticPackTest extends TestCase
{
    const FILE_DUMP_PATH = "dump/";
    private $packer = null;
    private $file;

    static private function toDumpDest($fileName): string
    {
        return self::FILE_DUMP_PATH.$fileName;
    }

    static private function assertFileContent($fileHandle, $content, string $message = "") : void
    {
        $read = fread($fileHandle, strlen($content));
        static::assertThat($read, new IsIdentical($content), $message);
    }

    protected function setUp()
    {
        parent::setUp();
        $this->file = fopen($this->toDumpDest("temp.otic"), "w");
        $this->packer = new OticPack($this->file);
    }

    protected function tearDown()
    {
        parent::tearDown();
        $this->packer->close();
        fclose($this->file);
    }

    public function test_creates()
    {
        $file = fopen($this->toDumpDest("file1.otic"), "w");
        $this->packer = new OticPack($file);

        $this->assertFileExists($this->toDumpDest("file1.otic"));
        $this->assertIsObject($this->packer);
        
        $this->packer->close();
        fclose($file);

        $file = fopen($this->toDumpDest("file1.otic"), "r");
        $this->assertFileContent($file, hex2bin("4fa946350001"));
        fclose($file);

        $this->assertSame(6, filesize($this->toDumpDest("file1.otic")));
    }

    public function test_createsChannel()
    {
        $channel = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        static::assertIsObject($channel);
        $interval = $channel->getTimeInterval();
        $this->assertIsArray($interval);
        $this->assertNull($interval[0]);
        $this->packer->flush();
        $this->packer->close();
    }

    public function test_createsMultipleChannels()
    {
        $channel1 = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        static::assertIsObject($channel1);

        $channel2 = $this->packer->defineChannel(0x02, OticPackChannel::TYPE_SENSOR, 0x00);
        static::assertIsObject($channel1);
        $channel3 = $this->packer->defineChannel(0x03, OticPackChannel::TYPE_SENSOR, 0x00);
        static::assertIsObject($channel1);
        $channel4 = $this->packer->defineChannel(0x04, OticPackChannel::TYPE_SENSOR, 0x00);
        static::assertIsObject($channel1);

        $channel1->close();
        $channel2->close();
        $channel3->close();
        $channel4->close();
    }

    public function test_assertInvalidMultipleChannels()
    {
        $channel1 = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $channel2 = null;
        try {
            $channel2 = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        } catch (LibOticException $e) {
            static::assertSame($e->getMessage(), "Invalid Argument");
        } finally {
            static::assertNull($channel2);
        }
        $channel1->close();
        $this->packer->clearErrorFlag();
        $channel2 = $this->packer->defineChannel(0x01, OticPackChannel::TYPE_SENSOR, 0x00);
        $channel2->close();
    }

    public function test_assertInvalidArgs()
    {
        try {
            $channel1 = $this->packer->defineChannel(0x01, 2, 0x00);
        } catch (OticException $e) {
            static::assertSame("Invalid Channel Type", $e->getMessage());
        }

        try {
            $channel1 = $this->packer->defineChannel(-1, OticPackChannel::TYPE_SENSOR, 0x00);
        } catch (OticException $e) {
            static::assertSame("Invalid ChannelID! Reason: Valid Range (int): 0 - 255", $e->getMessage());
        }
    }
}
