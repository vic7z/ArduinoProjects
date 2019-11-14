#include <ESP8266WiFi.h>


const char* ssid     = "spaceship";
const char* password = "88888888";


const char* resource = "/trigger/wifibutton/with/key/bUKA7qhASl1gyf_7OAYvTa8Tpj5ouZZbhK3Lq-o_Xsi";




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
Resource : <input type="text" name="res" maxlength="200">
<br>
	<input type="submit" value="Submit">
</pre>

</form>

</body>

</html>
)=====";







const char* server = "maker.ifttt.com";

void setup() {
  Serial.begin(9600); 

  initWifi();
  makeIFTTTRequest();

  
  ESP.deepSleep(0);
}

void loop() {

}


void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  

  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}


void makeIFTTTRequest() {
  Serial.print("Connecting to "); 
  Serial.print(server);
  
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if(!!!client.connected()) {
     Serial.println("Failed to connect, going back to sleep");
  }
  
  Serial.print("Request resource: "); 
  Serial.println(resource);
  client.print(String("GET ") + resource + 
                  " HTTP/1.1\r\n" +
                  "Host: " + server + "\r\n" + 
                  "Connection: close\r\n\r\n");
                  
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  
  if(!!!client.available()) {
     Serial.println("No response, going back to sleep");
  }
  while(client.available()){
    Serial.write(client.read());
  }
  
  Serial.println("\nclosing connection");
  client.stop();
}
