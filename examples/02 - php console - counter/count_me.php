<?php

require "../../clients/php/WhiteBellClient.php";

$whitebell = new \WhiteBell\Client("PHP console clock");
$whitebell->connectUNIX("../../whitebell.sock");

$whitebell->addCounterListener("scripts", function ($count){
    echo "Connected scripts: " . $count . "\n";
}); // Triggers immediately only for this script, to provide current count

usleep(100); // little delay, to be sure the message has arrived.
$whitebell->fetch(); // To receive the current count.

echo "Joining... " . "\n";
$whitebell->joinCounter("scripts"); // Modifies and triggers for all ascripts

$whitebell->run(); // Recieves the triggered counter events and calls callback.
