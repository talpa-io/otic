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
        $this->packer->defineChannel(1, OticPackChannel::TYPE_SENSOR, 0x00);
    }
}
