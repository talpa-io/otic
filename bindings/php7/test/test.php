<?php

    use Otic\OticPack;
    use Otic\OticUnpack;
    use Otic\Aggregator;
    use Otic\OticUnpackChannel;
    use Otic\LibOticException;
    use Otic\OticPackChannel;


//
//     $file = fopen("unittest.otic", "w");
//
//     $oticPack = new OticPack($file);
//     $channel123 = $oticPack->defineChannel(123,0,123);
//     $channel123->inject(1, "abc", "abc", 123);
//     $channel123->inject(1000, "abc", "abc", 123);
//     $channel123->inject(20000, "abc", "abc", 123);
//
//     $oticPack->close();
//
// //  echo "$channel123->getTimeInterval()[0] $channel123->getTimeInterval()[1]\n";
// //     var_dump($channel123->getTimeInterval());
//     fclose($file);
//     $val = $channel123->getTimeInterval();
//     echo "$val[0], $val[1]\n";
//
//
//     $file = fopen("unittest.otic", "r");
//     $oticUnpack = new OticUnpack($file);
//     $channel = $oticUnpack->selectChannel(123, function ($timestamp, $sensorName, $unit, $value) {
//         echo "test\n";
//     });
//
//     while (!feof($file))
//         $oticUnpack->parse();
//
//
//     var_dump($channel->getTimeInterval());



$file = fopen("unittest.otic", "w");
$oticPack = new OticPack($file);
$channel = $oticPack->defineChannel(0,0,0);
for($i = 0; $i<100000; $i++) {
    $channel->inject($i,"sensor".$i%19, "abc", $i%3);
}

echo $channel."\n";


$oticPack->close();
fclose($file);
$file = fopen("unittest.otic", "r");
$oticUnpack = new OticUnpack($file);
$data = [];
$channel = $oticUnpack->selectChannel(0, function ($timestamp, $sensorName, $unit, $value) use (&$data){
    $data[] = ['ts'=>$timestamp, 'name'=>$sensorName, 'unit'=>$unit, 'val'=>$value];
});
while (!feof($file)) {
    $oticUnpack->parse();
}
print_r($data[0]);
$time = $channel->getTimeInterval();
print_r($time);
$oticUnpack->close();
fclose($file);



//     $oticFile = fopen("out1.otic", "w");
//
//     $packStream = new OticPack($oticFile);
//     $sensorChannel1 = $packStream->defineChannel(0x01, 0, 0x00);
//     $sensorChannel2 = $packStream->defineChannel(0x02, 0, 0x00);
//
//     $sensorChannel1->inject(1234.4, "sensor1", "sensorUnit1", 1232434);
//     $sensorChannel1->inject(1234.5, "sensor2", "sensorUnit1", 12);
//     $sensorChannel1->inject(1234.5, "sensor3", "sensorUnit1", 1232434);
//     $sensorChannel1->inject(1234.5, "sensor2", "sensorUnit1", 123);
//     $sensorChannel1->inject(1234.5, "sensor1", "sensorUnit1", 1232434);
//     $sensorChannel1->inject(1234.5, "sensor3", "sensorUnit1", 1232434);
//
//     $sensorChannel1->close();
//     $packStream->close(); // or $packStream = null;
//     fclose($oticFile);
//
// //    Unpack
//     $oticFile = fopen("out1.otic", "r");
//     $outFile = fopen("out1.tsv", "w");
//
//     $unpacker = new OticUnpack($oticFile);
//     $outChannel = $unpacker->selectChannel(1, function ($timestamp, $sensorName, $unit, $value) {
//         echo "Test\n";
//     });
//
//     while (!feof($oticFile))
//         $unpacker->parse();
//     //$unpacker->read();
//
//     print_r($outChannel->getTimeInterval());
//
//
//     echo "Time Interval: Start -> ".$outChannel->getTimeInterval()[0].", End -> ".$outChannel->getTimeInterval()[1]."\n";
//
//     fclose($oticFile);
//     fclose($outFile);