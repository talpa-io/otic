<?php

    // throw new OticException("Some Test!");
    // throw new LibOticException(3);
    $fileOut = fopen("dump.otic", "w");
    $x = new OticPack($fileOut);
    $y = $x->defineChannel(1, 0, 1);

    for ($counter = 0; $counter < 1000000; $counter++)
        $y->inject($counter, "sensor1", "unit1", 12345);

    $y->inject(1000000, "sensor2", "unit2", "someString");
    $y->inject(1000000, "sensor2", "unit2", 123.34);
    $y->inject(1000000, "sensor2", "unit2", null);
    $y->inject(1000000, "sensor2", "unit2", "some other string");

    $x = null;
    fclose($fileOut);

