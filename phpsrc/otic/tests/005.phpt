--TEST--
OticReaderRaw exceptions are raised
--SKIPIF--
<?php
if (!extension_loaded('otic')) {
    echo 'skip';
}
?>
--FILE--
<?php
// check that problems raise exceptions, not errors
$r = new OticReaderRaw;
$r->open("tests/somedata.otic");
$r->close();
$r->read();
$r->read(); // never reached because of exception
?>
--EXPECT--
Fatal error: Uncaught Exception: reader not open! in /opt/phpsrc/otic/tests/005.php:6
Stack trace:
#0 /opt/phpsrc/otic/tests/005.php(6): OticReaderRaw->read()
#1 {main}
  thrown in /opt/phpsrc/otic/tests/005.php on line 6
