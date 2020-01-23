#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

//Web page from esp8266

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

const char* ssidap = "wifibutton";
const char* passap = "wifibuttontest";

String qsid = "";
String qpass = "";
String resourse = "";

void eepromwrit(void);
void webserver(void);

const char* serverre = "maker.iftt.com";

ESP8266WebServer server(80);

void setup(){
  
  Serial.begin(9600);
  EEPROM.begin(512);


//Reading the saved ssid

  String esid = "";
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);

//Reading the saved password

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
  

  WiFi.begin(esid.c_str(), epass.c_str());.   // Connecting to the wifi
  
  if(testWifi()){
    Serial.println("sucessfully connected");
    return;
  }
  else{
    Serial.print("turning on hotspot");
    webserver();
  }
    
  int timeout = 60 * 4; // 10 seconds

// Running the web page until establishing a connection

  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.println(".");
    server.handleClient();

  }

//Wifi details

   if(WiFi.status() != WL_CONNECTED) {
       Serial.println("not connected");
   }
     else{
       Serial.println("connected");
       Serial.print(millis());
       Serial.print(", IP address: "); 
       Serial.println(WiFi.localIP());
    }


}
// Nothing on void loop()
  
void loop(){
  
    }

// Writing data to the esps eeprom

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
        delay(150);
        ESP.reset();  //resetting the esp after writing the credentials  and triggers the iftt
}



//Turning on the web server

void webserver(){

  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap,passap);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("disconnect from the WiFi ");
  delay(10);
  Serial.print("Local IP: ");
  Serial.println(IP);
  
server.on("/",root);
server.on("/action",handlepage);
server.begin();
Serial.println("server started");
  
}


//Checking the wifi connection

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

//Web page handler

void handlepage(){

qsid = server.arg("ssid");
qpass = server.arg("passwd");
resourse = server.arg("res");
Serial.println("data from the web");

Serial.println(qsid);
Serial.println(qpass);
Serial.println(resourse);

eepromwrit();
}


void root(){
    String page = MAIN_page;
    server.send(200,"text/html",page);
  }
