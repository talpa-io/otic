#!/usr/bin/php

<?php
use Otic\OticPackStream;
use Otic\OticPackChannel;

// TODO: Bind C's is_numeric function. Reasons: faster and more reliable
class TSVParser
{
    public function __construct()
    {
    }

    public function run($argc, $argv)
    {
        if ($argc === 1) {
            echo $this->help();
        }
        if ($argc === 6) {
            if ($argv[1] === '-p') {
                if ($argv[2] === "-i" && $argv[4] === "-o") {
                    $inputFile = fopen($argv[3], 'r');
                    $outputFile = fopen($argv[5], 'wb');
                    $packer = new OticPackStream($outputFile);
                    $channel = $packer->defineChannel(0x01, "sensor", 0x00);
                    $lineContent = [];
                    $counter = 0;
                    while (!feof($inputFile))
                    {
                         $lineContent = explode("\t", stream_get_line($inputFile, 512, "\n"));
                         if (count($lineContent) != 5) {
                            continue;
                         } else if (is_numeric($lineContent[4])) {
                            if (strstr($lineContent[4], '.') === false)
                                $channel->inject(floatval($lineContent[0]), $lineContent[3], $lineContent[3], intval($lineContent[4]));
                            else
                                $channel->inject(floatval($lineContent[0]), $lineContent[3], $lineContent[3], floatval($lineContent[4]));
                         } else if ($lineContent[4] == "") {
                            $channel->inject(floatval($lineContent[0]), $lineContent[3], $lineContent[3], null);
                         } else {
                            $channel->inject(floatval($lineContent[0]), $lineContent[1], $lineContent[3], $lineContent[4]);
                         }
                         $counter++;
                    }
                    $packer = null;
                    fclose($inputFile);
                    fclose($outputFile);
                    return 0;
                }
            } else if ($argv[1] === '-u') {
                echo "Unpack\n";
                return 0;
            }
        }
        throw new Error("Invalid Input!\n".$this->help());
    }

    public function help()
    {
        return "Usage: ./tsvParser.php [-p|-u|-h] -i <fileInName> -o <fileOutName>\n";
    }

    public function __destruct()
    {
    }
}

$x = new TSVParser();
//$x->run($argc, $argv);
$x->run(6, ["tsvParser", "-p", "-i", "bigFile.txt", "-o", "dump.otic"]);