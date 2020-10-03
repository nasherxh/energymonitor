///copy code from https://create.arduino.cc/projecthub/Blue_jack/personal-weather-station-arduino-esp8266-thingspeak-8d5cba

#include <SoftwareSerial.h>
#include <stdlib.h>

#define DEBUG 1   // change value to 1 to enable debuging using serial monitor 
String network = "Natashaphone" ;    // "SSID-WiFiname"
String password = "smartvils"; // "password"
#define IP "184.106.153.149"      // thingspeak.com ip

// RX, TX (Hardware serial Rx and Tx pins on Arduino (0,1) are needed for serial communication.
String GET = "GET /update?key=F9Q1ROVIZW8TLDYE"; //change it with your api key like "GET /update?key=Your Api Key"
//Hardware serial Rx and Tx pins on Arduino (0,1) are needed for serial communication.
//Use SoftwareSerial to enable Rx, Tx comms alongside Hardware Serial pins.
SoftwareSerial esp8266Module(2,3);  //(Rx, Tx)


//Variables
int hum =0;
float base=0;
String humC;
String baseC;
int error;


void setup()
{
  //if first time using ESP module, should change baud rate to 9600 instead of 1155200
//    Serial.begin(115200); //or use default 115200.
//    esp8266Module.begin(115200);
//    esp8266Module.println(F("AT+UART_DEF=9600,8,1,0,0"));
//    delay(4000);
  
  Serial.begin(9600);
  esp8266Module.begin(9600);
  delay(4000);
  
}

void loop()
{
    if(DEBUG){
    Serial.begin(9600);                             // Setting hardware serial baud rate to 9600
    }  
    setupEsp8266();
    hum = hum +1;
    base  = base + (float)hum;
    Serial.print(hum);
    Serial.print(",");
    Serial.println(base);
    updateThingSpeak(String(hum), String(base));
    //Resend if transmission is not completed
 
  delay(5000);
}


//-------------------------------------------------------------------
// Following function setup the esp8266, put it in station made and 
// connect to wifi access point.
//------------------------------------------------------------------
void setupEsp8266()                                   
{
    if(DEBUG){
      Serial.println("Reseting esp8266");
    }
    esp8266Module.flush();
    esp8266Module.println(F("AT+RST"));


    //PrintStringFromEspModule(); Useful for debugging but note .find("OK") won't work.  
      
    delay(7000);
    Serial.println("Finished reseting");
    if (esp8266Module.find("OK"))
    {
      if(DEBUG){
        Serial.println("Found OK");
        Serial.println("Changing espmode");
      }  
      
      changingMode();
      esp8266Module.flush();
      connectToWiFi();
    }
    else
    {
      if(DEBUG){
        Serial.println("OK not found");
      }
    }
}

//-------------------------------------------------------------------
// Following function sets esp8266 to station mode
//-------------------------------------------------------------------
bool changingMode()
{   
    esp8266Module.flush();
    esp8266Module.println(F("AT+CWMODE=1"));
    delay(5000);
    if (esp8266Module.find("OK"))
    {
      if(DEBUG){
        Serial.println("Mode changed");
      }  
      return true;
    }
    else
    {
      if(DEBUG){
        Serial.println("Error while changing mode");
      }  
      return false;
    }
}

//-------------------------------------------------------------------
// Following function connects esp8266 to wifi access point
//-------------------------------------------------------------------
bool connectToWiFi()
{
  if(DEBUG){
    Serial.println("inside connectToWiFi");
  }  
  String cmd = F("AT+CWJAP=\"");
  cmd += network;
  cmd += F("\",\"");
  cmd += password;
  cmd += F("\"");
  esp8266Module.println(cmd);
  PrintStringFromEspModule();  //useful for debugging, but note esp8266Module.find("OK") won't work if it is read and printed here first.
  delay(15000);
  if (esp8266Module.find("OK"))
  {
    if(DEBUG){
      Serial.println("Connected to Access Point");
    }  
    return true;
  }
  else
  {
    if(DEBUG){
      Serial.println("Could not connect to Access Point");
    }  
    return false;
  }
}

//-------------------------------------------------------------------
// Following function sends sensor data to thingspeak.com
//-------------------------------------------------------------------
void updateThingSpeak(String voltage1,String voltage2)
{  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  esp8266Module.println(cmd);
  PrintStringFromEspModule();
  delay(5000);
  if(esp8266Module.find("ERROR")){
    if(DEBUG){
      Serial.println("ERROR while SENDING");
    }  
    return;
  }
  cmd = GET + "&field1=" + voltage1 + "&field2=" + voltage2 + "\r\n";
  esp8266Module.print("AT+CIPSEND=");
  esp8266Module.println(cmd.length());
  delay(15000);
  if(esp8266Module.find(">"))
  {
    esp8266Module.print(cmd);
    if(DEBUG){
      Serial.println("Data sent");
    }
  }
  else
  {
    esp8266Module.println("AT+CIPCLOSE");
    if(DEBUG){
      Serial.println("Connection closed");
    }  
  }
}


void PrintStringFromEspModule()
{
  long int time = millis();                                         //get the operating time at this specific moment and save it inside the "time" variable.      
  String response = "";   
  while( (time+3000) > millis())                                 //excute only whitin 1 second.      
  {            
    while(esp8266Module.available())                                      //is there any response came from the ESP8266 and saved in the Arduino input buffer?      
    {      
       char c = esp8266Module.read();                                      //if yes, read the next character from the input buffer and save it in the "response" String variable.      
       response+=c;                                                  //append the next character to the response variabl. at the end we will get a string(array of characters) contains the response.      
    }        
  }          
  if(DEBUG)                                                         //if the "debug" variable value is TRUE, print the response on the Serial monitor.      
  {      
    Serial.print(response);      
   } 
}         
