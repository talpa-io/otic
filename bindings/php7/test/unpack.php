<?php
    use Otic\OticUnpack;
    use Otic\OticUnpackChannel;


    $outputFile = fopen("someFile.txt", "w");
    function flusher($timestamp, $sensorName, $sensorUnit, $value)
    {
        global $outputFile;
        fwrite($outputFile, "$timestamp\t$sensorName\t$sensorUnit\t$value\n");
    }

    $fileIn = fopen("out.otic", "r");
    $x = new OticUnpack($fileIn);

    $y = $x->selectChannel(1, 'flusher');
//     $z = $x->selectChannel(3, function($ts, $sn, $su, $val) {echo "$ts\t$sn\t$su\t$val\n"; });

//     $y->setFetchList("sensor1");
//     $counter = 0;
//     while (1)
//     {
//         $z = $x->generate();
//         if ($z === null)
//             break;
//         var_dump($z);
//     };

    while (!feof($fileIn)) {
        $x->parse();
    }
//
    $x->close();
    fclose($fileIn);