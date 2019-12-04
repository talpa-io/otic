<?php
    use Otic\OticPack;
    use Otic\OticPackChannel;
    use Otic\OticPackStream;


    $outFile = fopen('dump2.otic', 'wb');
    $y = new OticPackStream($outFile);
    $channel = $y->defineChannel(0x01, "sensor", 0x00);

    for ($counter = 0; $counter < 1000000; $counter++)
    {
        $channel->inject($counter, "sensor1", "unit1", 123);
    }

    //$otherChannel->inject(1243, "sensor1", "sensorUnit1", 123);             // Long
    //$otherChannel->inject(1243, "sensor2", "sensorUnit2", null);            // null
    //$otherChannel->inject(1243, "sensor2", "sensorUnit2", 1.2);             // Double
    //$otherChannel->inject(1243, "sensor2", "sensorUnit2", "Testing");       // String

    $y = null;
    fclose($outFile);