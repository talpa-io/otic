<?php
    use Otic as Otic;
    function my_sum($a, $b) {
        return $a + $b;
    }
    $x = new Otic\OticPack();
    echo "Returned: ".$x->test('my_sum')."\n";
//    var_dump($x->test)
    $x = null;
	echo "Hallo World\n";