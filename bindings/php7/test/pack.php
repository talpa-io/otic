<?php

    // throw new OticException("Some Test!");
    // throw new LibOticException(3);
    $fileOut = fopen("dump.otic", "w");
    $x = new OticPack($fileOut);
    $y = $x->defineChannel(1, 0, 1);
    $z = $x->defineChannel(2, 0, 1);

    for ($counter = 0; $counter < 1000000; $counter++)
        $z->inject($counter, "sensor1", "unit1", 12345);

//     $z->inject(1000000, "sensor2", "unit2", "someString");
//     $z->inject(1000000, "sensor2", "unit2", 123.34);
//     $z->inject(1000000, "sensor2", "unit2", null);
//     $z->inject(1000000, "sensor2", "unit2", "some other string");

    $x->close = null;
    fclose($fileOut);
