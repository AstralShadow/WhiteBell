<?php

require "../../clients/php/WhiteBellClient.php";

$whitebell = new \WhiteBell\Client("PHP console clock");
$whitebell->connectUNIX("../../whitebell.sock");

while (1){
    $time = time();
    $whitebell->dispatchEvent("clock", $time);
    echo "Sent time: " . $time . "\n";

    sleep(1);
}