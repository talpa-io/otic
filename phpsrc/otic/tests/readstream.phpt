--TEST--
OticReaderRaw read stream
--SKIPIF--
<?php
if (!extension_loaded('otic')) {
    echo 'skip';
}
?>
--FILE--
<?php
$r = new OticReaderRaw;
$s = fopen("tests/somedata.otic", "rb");
$r->open_stream($s);
for ($x = 0; $x < 10; $x++) {
    var_dump($r->read());
}
$r->close();
?>
--EXPECT--
array(3) {
  ["colname"]=>
  string(3) "abc"
  ["ts"]=>
  float(1.1)
  ["value"]=>
  int(1233322)
}
array(3) {
  ["colname"]=>
  string(3) "def"
  ["ts"]=>
  float(1.1)
  ["value"]=>
  int(-1233322)
}
array(3) {
  ["colname"]=>
  string(3) "abc"
  ["ts"]=>
  float(1.2)
  ["value"]=>
  int(1233322)
}
array(3) {
  ["colname"]=>
  string(3) "def"
  ["ts"]=>
  float(1.3)
  ["value"]=>
  int(-1233322)
}
array(3) {
  ["colname"]=>
  string(6) "abcdef"
  ["ts"]=>
  float(5.3)
  ["value"]=>
  float(5.5)
}
array(3) {
  ["colname"]=>
  string(6) "abcdef"
  ["ts"]=>
  float(6)
  ["value"]=>
  string(5) "hello"
}
array(4) {
  ["colname"]=>
  string(6) "abcdef"
  ["metadata"]=>
  string(8) "baaaaaaa"
  ["ts"]=>
  float(6)
  ["value"]=>
  int(9)
}
array(4) {
  ["colname"]=>
  string(6) "abcdef"
  ["metadata"]=>
  string(8) "baaaaaaa"
  ["ts"]=>
  float(6)
  ["value"]=>
  NULL
}
array(4) {
  ["colname"]=>
  string(6) "abcdef"
  ["metadata"]=>
  string(8) "baaaaaaa"
  ["ts"]=>
  float(6.5)
  ["value"]=>
  NULL
}
bool(false)
