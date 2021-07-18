#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WhiteBellClient.hpp>

WhiteBell::Client *bell;

char const* server_ip = "192.168.0.0"; // set your WhiteBell server ip here.

void on_my_event(string control){
    Serial.print("led: ");
    Serial.println(control.c_str());

    if(control == "on")
        digitalWrite(LED_BUILTIN_AUX, LOW);
    else if(control == "off")
        digitalWrite(LED_BUILTIN_AUX, HIGH);
}

void WiFi_await_connected(){
    while (WiFi.status() != WL_CONNECTED)
        delay(250);
}

void setup()
{
    pinMode(LED_BUILTIN_AUX, OUTPUT);
    Serial.begin(9600);

    const char ssid[] = "My WiFi"; // your SSID
    const char pass[] = "My Pass"; // your password
    WiFi.begin(ssid, pass);
    WiFi_await_connected();
    bell = new WhiteBell::Client(server_ip, 25250);
    bell->set_namespace("arduino test");
    bell->track_event("led", on_my_event);
    Serial.println("ready");
}


void loop()
{
    bell->fetch();
}
