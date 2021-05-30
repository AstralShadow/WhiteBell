<?php

require "../../clients/php/WhiteBellClient.php";

$whitebell = new \WhiteBell\Client("../../whitebell.sock", "PHP console clock");

$whitebell->addEventListener("clock", function ($time){
    echo "Received time: " . $time . "\n";
});

$whitebell->run();
