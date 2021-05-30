<?php

require "../../clients/php/WhiteBellClient.php";

$whitebell = new \WhiteBell\Client("../../whitebell.sock", "PHP console clock");

while (1){
    $time = time();
    $whitebell->dispatchEvent("clock", $time);
    echo "Sent time: " . $time . "\n";

    sleep(1);
}