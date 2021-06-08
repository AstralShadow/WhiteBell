<?php

require "../../clients/php/WhiteBellClient.php";

$whitebell = new \WhiteBell\Client("PHP console clock");
$whitebell->connectUNIX("../../whitebell.sock");

$whitebell->addEventListener("clock", function ($time){
    echo "Received time: " . $time . "\n";
});

$whitebell->run();
