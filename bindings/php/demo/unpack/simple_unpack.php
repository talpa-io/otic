<?php
    use Otic\OticUnpack;

    $inputFile = fopen('dump.otic', "rb");






    //function fetcher($size)
    //{
    //    global $inputFile;
    //    return fread($inputFile, $size);
    //}

    //function seeker($offset)
    //{
    //    global $inputFile;
    //    return fseek($inputFile, $offset, SEEK_CUR) == 0;
    //}

    //function flusher1($content, $size)
    //{
    //    echo $content."\n";
    //}

    //$a = new OticUnpack('fetcher');

    //$a->defineChannel(1, 'flusher1');

    //$a->parse();

    fclose($inputFile);