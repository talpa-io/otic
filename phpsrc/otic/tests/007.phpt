--TEST--
OticReaderRaw get_closing_timestamp errors before EOF
--SKIPIF--
<?php
if (!extension_loaded('otic')) {
    echo 'skip';
}
?>
--FILE--
<?php
$r = new OticReaderRaw;
$r->open("tests/somedata.otic");
var_dump($r->read());
var_dump($r->get_closing_timestamp());
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

Fatal error: Uncaught Exception: reader not at end! in /opt/phpsrc/otic/tests/007.php:5
Stack trace:
#0 /opt/phpsrc/otic/tests/007.php(5): OticReaderRaw->get_closing_timestamp()
#1 {main}
  thrown in /opt/phpsrc/otic/tests/007.php on line 5
