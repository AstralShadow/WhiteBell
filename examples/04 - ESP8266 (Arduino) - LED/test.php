<?php

include "/workspace/c++/WhiteBell/clients/php/WhiteBellClient.php";
$client = new WhiteBell\Client("arduino test");
$client->connectTCP("127.0.0.1", 25250);

while(1){
	echo "led on\n";
	$client->dispatchEvent("led", "on");
	usleep(500000);

	echo "led off\n";
	$client->dispatchEvent("led", "off");
	usleep(500000);
}


