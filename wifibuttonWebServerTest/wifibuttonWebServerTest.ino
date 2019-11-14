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
Resource : <input type="text" name="res" maxlength="200">
<br>
	<input type="submit" value="Submit">
</pre>

</form>

</body>

</html>
)=====";



String qsid = "";
String qpass = "";
String resourse = "";

void eepromwrit(void);
void webserver(void);

const char* server = "maker.iftt.com";

ESP8266WebServer server(69);

void setup(){
  
  Serial.begin(9600);
  EEPROM.begin(512);

  String esid = "";
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  Serial.println("Reading EEPROM resourse");

  for (int i = 96; i <160 ; i++)
  {
    resourse += char(EEPROM.read(i));
  }
  

  WiFi.begin(esid.c_str(), epass.c_str());
    
  int timeout = 10 * 4; // 10 seconds

  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.println(".");
  }

   if(WiFi.status() != WL_CONNECTED) {

   }
     else{
       Serial.println("connected");
       Serial.print(millis());
       Serial.print(", IP address: "); 
       Serial.println(WiFi.localIP());
    }


}

  
  void loop(){
  
    }


void eepromwrit(){

    Serial.println("writing eeprom SSID:");

    for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");

        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }

        Serial.println("writing eeprom resourse:");

        for (int i = 0; i < resourse.length(); ++i)
        {
          EEPROM.write(96 + i, resourse[i]);
          Serial.print("Wrote: ");
          Serial.println(resourse[i]);
        }


        EEPROM.commit();

}


void webserver(){
  WiFi.disconnect();
  Serial.println("disconnect from the WiFi ");
  delay(10);
  
}