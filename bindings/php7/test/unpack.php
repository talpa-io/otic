<?php
    use Otic\OticUnpack;
    use Otic\OticUnpackChannel;


    $outputFile = fopen("someFile.txt", "w");
    function flusher($timestamp, $sensorName, $sensorUnit, $value)
    {
        global $outputFile;
//         echo "Called";
        fwrite($outputFile, "$timestamp\t$sensorName\t$sensorUnit\t$value\n");
    }

    $fileIn = fopen("dump.otic", "r");
    $x = new OticUnpack($fileIn);

    $y = $x->selectChannel(2, 'flusher');

//     $y->setFetchList("sensor1");
    $counter = 0;
    while (1)
    {
        $z = $x->generate();
        if ($z === null)
            break;
        var_dump($z);
    };

//     while (!feof($fileIn)) {
//         $x->parse();
//     }
//
    $x->close();
    fclose($fileIn);