--TEST--
Check if otic is loaded
--SKIPIF--
<?php
if (!extension_loaded('otic')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "otic" is available';
?>
--EXPECT--
The extension "otic" is available
