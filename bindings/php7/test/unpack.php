<?php


    $fileIn = fopen("dump.otic", "r");
    $x = new OticUnpack($fileIn);




    $x = null;
    fclose($fileIn);