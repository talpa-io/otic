<?php

    use Otic\OticPack;

    $fileOut = fopen("dump.otic", "w");
    $x = new OticPack($fileOut);
    $y = $x->defineChannel(1, 0, 1);
    $z = $x->defineChannel(2, 0, 1);

    $z->inject(1000000, "sensor2", "unit2", "someString");
    $z->inject(1000000, "sensor2", "unit2", 123.34);
    $z->inject(1000000, "sensor2", "unit2", null);
    $z->inject(1000000, "sensor2", "unit2", "some other string");

    var_dump($z->getTimeInterval());

    $x->close();

    fclose($fileOut);
