<?php
    use Otic\OticPack;

    $outputFile = fopen('dump.otic', 'wb');

    $x = new OticPack(function($content, $size) use ($outputFile){ fwrite($outputFile, $content,$size); });
    $x->defineChannel(1, "sensor", 0);

    //$y = new OticPackChannel();


    fclose($outputFile);