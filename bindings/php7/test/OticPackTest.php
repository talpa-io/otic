<?php

declare(strict_types = 1);

namespace Tests;

use Otic\OticPack;
use phpDocumentor\Reflection\Types\Void_;
use PHPUnit\Framework\TestCase;

final class OticPackTest extends TestCase
{
    public function testCanCreateFile(): void
    {
        $this->assertFalse(1 == 0);
    }
}
