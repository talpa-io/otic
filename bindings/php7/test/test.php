<?php

    $oticFile = fopen("out.otic", "w+");

    $packStream = new OticPack($oticFile);

    $sensorChannel = $packStream->defineChannel(1, 0, 0x00);

    for ($i=0; $i<1000000; $i++) {
        $sensorChannel->inject($i, "sensor1", "unit1", 1234);
    }

    $packStream = null;
    fclose($oticFile);


    $oticFile = fopen("out.otic", "r");
    $outFile = fopen("out.tsv", "w+");

    $unpacker = new OticUnpack($oticFile);
    function writer ($ts, $sn, $su, $val) {
        global $outFile;
        fwrite($outFile, "$ts;$sn;$su;$val\n");
    }
    $unpacker->selectChannel(1, 'writer');

    while ( ! feof($oticFile))
        $unpacker->parse();

    fclose($oticFile);
    fclose($outFile);



