<?php

    use Otic\OticPack;
    use Otic\OticPackChannel;

    $fileOut = fopen("dump.otic", "w");
    $x = new OticPack($fileOut);
    $y = $x->defineChannel(1, OticPackChannel::TYPE_SENSOR, 0x00);
    $z = $x->defineChannel(2, OticPackChannel::TYPE_BINARY, 0x00);

    $z->inject(1000000, "sensor2", "unit2", "someString");
    $z->inject(1000000, "sensor2", "unit2", 123.34);
    $z->inject(1000000, "sensor2", "unit2", null);
    $z->inject(1000000, "sensor2", "unit2", "some other string");


    var_dump($y->getStats());
    var_dump($z->getTimeInterval());

    var_dump($z->getStats());


    $x->close();

    fclose($fileOut);
