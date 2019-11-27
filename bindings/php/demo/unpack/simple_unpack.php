<?php
    use Otic\OticUnpack;
    use Otic\OticUnpackStream;

    $inputFile = fopen('dump2.otic', 'rb');
    $outputFile = fopen('dump2.txt', 'w');

    $unpacker = new OticUnpackStream($inputFile);
    $unpacker->defineChannel(0x01, $outputFile);

    while (!feof($inputFile))
        $unpacker->parse();

    $unpacker = null;
    fclose($outputFile);


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