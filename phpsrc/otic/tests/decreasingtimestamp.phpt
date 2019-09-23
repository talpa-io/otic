--TEST--
OticWriter errors on decreasing timestamp
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
$w->write($c, 1.4, -23.4);
$w->close();
?>
--EXPECT--
Fatal error: Uncaught Exception: timestamp decreased! in /opt/phpsrc/otic/tests/decreasingtimestamp.php:7
Stack trace:
#0 /opt/phpsrc/otic/tests/decreasingtimestamp.php(7): OticWriterRaw->write(0, 1.4, -23.4)
#1 {main}
  thrown in /opt/phpsrc/otic/tests/decreasingtimestamp.php on line 7
