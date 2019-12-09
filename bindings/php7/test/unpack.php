<?php

    $outputFile = fopen("someFile.txt", "w");
    function flusher($timestamp, $sensorName, $sensorUnit, $value)
    {
        global $outputFile;
        fwrite($outputFile, "$timestamp\t$sensorName\t$sensorUnit\t$value\n");
    }
    $fileIn = fopen("dump.otic", "r");
    $x = new OticUnpack($fileIn);
    $y = $x->selectChannel(1, 'flusher');
    $y->setFetchList("sensor1");
    while (!feof($fileIn)) {
        $x->parse();
    }

    $x = null;
    fclose($fileIn);