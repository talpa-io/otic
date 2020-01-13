<?php

    use Otic\OticPack;
    use Otic\OticUnpack;
    use Otic\Aggregator;
    use Otic\OticUnpackChannel;
    use Otic\LibOticException;

    $oticFile = fopen("out1.otic", "w");

    $packStream = new OticPack($oticFile);
    $sensorChannel1 = $packStream->defineChannel(0x01, 0, 0x00);
    $sensorChannel2 = $packStream->defineChannel(0x02, 0, 0x00);

    $sensorChannel1->inject(1234.4, "sensor1", "sensorUnit1", 1232434);
    $sensorChannel1->inject(1234.5, "sensor2", "sensorUnit1", 12);
    $sensorChannel1->inject(1234.5, "sensor3", "sensorUnit1", 1232434);
    $sensorChannel1->inject(1234.5, "sensor2", "sensorUnit1", 123);
    $sensorChannel1->inject(1234.5, "sensor1", "sensorUnit1", 1232434);
    $sensorChannel1->inject(1234.5, "sensor3", "sensorUnit1", 1232434);

    echo "Time Interval: ".$sensorChannel1->getTimeInterval()[0].", ".$sensorChannel1->getTimeInterval()[1].".\n";

    $sensorChannel1->close();
    $packStream->close(); // or $packStream = null;
    fclose($oticFile);


//    Unpack
    $oticFile = fopen("out1.otic", "r");
    $outFile = fopen("out1.tsv", "w+");

    $unpacker = new OticUnpack($oticFile);
    function writer ($ts, $sn, $su, $val) {
        global $outFile;
        echo "$ts\t$sn\t$su\t$val\n";
        fwrite($outFile, "$ts;$sn;$su;$val\n");
    }

    //$unpacker->selectChannel(1, 'writer'); or
    $outChannel = $unpacker->selectChannel(1, function($ts, $sn, $su, $val) { global $outFile; fwrite($outFile, "$ts;$sn;$su;$val\n"); echo "$ts\t$sn\t$su\t$val\n";});
    $outChannel->setFetchList("sensor2");

    while (!feof($oticFile))
        $unpacker->parse();
    //$unpacker->read();
    echo "Time Interval: Start -> ".$outChannel->getTimeInterval()[0].", End -> ".$outChannel->getTimeInterval()[1]."\n";

    fclose($oticFile);
    fclose($outFile);