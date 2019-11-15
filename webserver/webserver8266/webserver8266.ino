#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>


const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>

<html>

<body>
 
<h2> Form for ESP8266 </h2>


<form action="/action">
<br>
<pre>
SSID     : <input type="text" name="ssid" maxlength="32">
<br>
Password : <input type="password" name="passwd" maxlength="32">
<br>
Resource : <input type="text" name="res" maxlength="100">
<br>
	<input type="submit" value="Submit">
</pre>

</form>

</body>

</html>
)=====";

const char* ssid ="wifibutton";
const char* pass ="wifibuttontest";

ESP8266WebServer server(80);

void handleroot(){
    String root = MAIN_page;
    server.send(200,"text/html",root);

}


void setup(){
    Serial.begin(9600);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid,pass);
    IPAddress IP=WiFi.softAPIP();
    Serial.println(IP);
    server.on("/",handleroot);
    server.begin();
    while(true){
        server.handleClient();
    }
}

void loop(){

}
