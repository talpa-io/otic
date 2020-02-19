<?php

declare(strict_types=1);

namespace Otic;
use PHPUnit\Framework\TestCase;
use Otic\OticException;

final class OticExceptionTest extends TestCase
{
    public function test_construct()
    {
        $this->expectException(OticException::class);
        throw new OticException("Something");
    }
}