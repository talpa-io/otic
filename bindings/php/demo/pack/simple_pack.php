<?php
    use Otic\OticPack;
    use Otic\OticPackChannel;

    $outputFile = fopen('dump.otic', 'wb');

    function test($content, $size, $data)
    {
        //return fwrite($data, $content, $size) != false;
    }

    $x = new OticPack('test', $outputFile);
    $channel = $x->defineChannel(0x01, "sensor", 0);

    //function test($content, $size)
    //{
    //    global $outputFile;
    //    fwrite($outputFile, $content, $size) != false;
    //}
    //$x = new OticPack('test');
    //$channel = $x->defineChannel(0x01, "sensor", 0x00);
    //$channel->inject(123456789, "sensorName1", "sensorUnit1", 1.2);
    //$channel->inject(123456789, "sensorName1", "sensorUnit1", 1.2);
    //$channel->close();

    //fclose($outputFile);

    //$channel->inject(123456789, "sensorName1", "sensorUnit1", 1.2);
    //$channel->inject(123456789, "sensorName1", "sensorUnit1", 1.2);
    //$channel->flush();
    //$x = new OticPack(function($content, $size) use ($outputFile){ fwrite($outputFile, $content, $size);});
    //$z = $x->defineChannel(1, "sensor", 0);
    //$z->flush();
    //$z->defineChannel(2, "sensor", 0);

    //$z->inject(1234, "sensorName1", "sensorUnit1", 1.2);
    //$z->flush();
    //$z->defineChannel(2, "sensor", 1);
    //$y = new OticPackChannel();
    //$y->flush();