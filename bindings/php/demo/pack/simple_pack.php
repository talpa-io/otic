<?php
    use Otic\OticPack;
    use Otic\OticPackChannel;
    use Otic\OticPackStream;

//    $outputFile = fopen('dump.otic', 'wb');

//    function test($content, $size, $data)
//    {
        //return fwrite($data, $content, $size) != false;
//    }


    //$x = new OticPack('test', $outputFile);
    //$channel = $x->defineChannel(0x01, "sensor", 0);


    $outFile = fopen('dump2.otic', 'wb');
    $y = new OticPackStream($outFile);
    $otherChannel = $y->defineChannel(0x01, "sensor", 0x00);

    $otherChannel->inject(1243, "sensor1", "sensorUnit1", 123);             // Long
    $otherChannel->inject(1243, "sensor2", "sensorUnit2", null);            // null
    $otherChannel->inject(1243, "sensor2", "sensorUnit2", 1.2);             // Double
    $otherChannel->inject(1243, "sensor2", "sensorUnit2", "Testing");       // String

    $y = null;
    fclose($outFile);

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