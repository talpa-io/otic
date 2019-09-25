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
$c2 = $w->define_column("bar");
$w->write($c, 51.5, 5);
$w->write($c, 52, -23.4);
$w->write($c, 545.5, "ABC");
$w->write($c, 545.5, null);
$stats = $w->close();
var_dump($stats);
?>
--EXPECT--
array(7) {
  ["num_rows"]=>
  int(8)
  ["num_bytes"]=>
  int(82)
  ["num_bytes_ts"]=>
  int(28)
  ["num_bytes_values"]=>
  int(40)
  ["num_ts_shifts"]=>
  int(5)
  ["num_blocks"]=>
  int(1)
  ["columns"]=>
  array(2) {
    ["foo"]=>
    array(2) {
      ["num_rows"]=>
      int(8)
      ["num_bytes"]=>
      int(40)
    }
    ["bar"]=>
    array(2) {
      ["num_rows"]=>
      int(0)
      ["num_bytes"]=>
      int(0)
    }
  }
}
