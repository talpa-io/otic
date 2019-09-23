--TEST--
OticWriterRaw basic test
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
$w->close();

$r = new OticReaderRaw;
$r->open("tests/writedata.otic");
var_dump($r->read());
var_dump($r->read());
var_dump($r->read());
var_dump($r->read());
?>
--EXPECT--
array(3) {
  ["colname"]=>
  string(3) "foo"
  ["ts"]=>
  float(1.5)
  ["value"]=>
  int(5)
}
array(3) {
  ["colname"]=>
  string(3) "foo"
  ["ts"]=>
  float(2)
  ["value"]=>
  float(-23.4)
}
array(3) {
  ["colname"]=>
  string(3) "foo"
  ["ts"]=>
  float(45.5)
  ["value"]=>
  string(3) "ABC"
}
array(3) {
  ["colname"]=>
  string(3) "foo"
  ["ts"]=>
  float(45.5)
  ["value"]=>
  NULL
}