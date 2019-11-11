<?php
    use Otic\OticPack;
    use Otic\OticPackChannel;

    $outputFile = fopen('dump.otic', 'wb');


    function test($content, $size)
    {
        echo "Called\n";
    }

    //$x = new OticPack(function($content, $size) use ($outputFile){ fwrite($outputFile, $content, $size);});
    $x = new OticPack('test');
    //$z = $x->defineChannel(1, "sensor", 0);
    echo $x;
//    $z->flush();
    //$z->defineChannel(2, "sensor", 0);



    //$z->inject(1234, "sensorName1", "sensorUnit1", 1.2);
//    $z->flush();
    //$z->defineChannel(2, "sensor", 1);
    //$y = new OticPackChannel();
    //$y->flush();

    fclose($outputFile);