--TEST--
OticWriterRaw test stats on closing writer
--SKIPIF--
<?php
if (!extension_loaded('otic')) {
    echo 'skip';
}
?>
--FILE--
<?php
$w = new OticWriterRaw;
$w->open("tests/writedata.otic");
$w->columns = [];
$c = $w->define_column("foo");
$w->write($c, 1.5, 5);
$w->write($c, 2, -23.4);
$w->write($c, 45.5, "ABC");
$w->write($c, 45.5, null);
$stats = $w->close();
var_dump($stats);
?>
--EXPECT--
array(6) {
  ["num_rows"]=>
  int(4)
  ["num_bytes"]=>
  int(64)
  ["num_bytes_ts"]=>
  int(10)
  ["num_bytes_values"]=>
  int(20)
  ["num_ts_shifts"]=>
  int(2)
  ["num_blocks"]=>
  int(1)
}

