//Set up SD Card
  // include the SD library:
  #include <SPI.h>
  #include <SD.h>  
  /*The circuit:
    * SD card attached to SPI bus as follows:
   ** MOSI - pin 11 on Arduino
   ** MISO - pin 12 on Arduino
   ** CLK - pin 13 on Arduino
   ** CS - Pin 4 used here for consistency with other Arduino examples. Pin 10 can also be used?
  */
  const int chipSelect = 4; 
  //SD card filename format
    char filename[] = "DATA00.CSV";
    bool readData=false;
    File dataFile;
  
//set up for analogue reading
  const int arduinoVCC = 5; //Arduino Voltage
  const unsigned int inputResolution = 1023;

//Set up data logging
  const unsigned int average_of = 500; //Take average of 500 readings for DC current & Voltage
  unsigned long previousMillis=0;

//Set up for dc current sensing
  const int dcCurrentAnalogPin = A1;
  const int CurrentSensorRating = 100;
  float dcCurrent;
  float dcCurrent_average;

//set up for dc voltage reading
  unsigned long ValueR1 = 10000;
  unsigned long ValueR2 = 1000;
  const int voltageAnalogPin = A0;
  float voltage;
  float dcVoltage_average;

////set up for AC current reading
  #include "EmonLib.h"
  EnergyMonitor emon1;
  const int ACCurrentAnalogPin = 2;
  const int ACSensorCalibration = 30; // Current want to read when 1V produced
  const int ACVoltage = 230.0; //rms voltage of thing sensing, used for apparent power calc
  double apparentPow;
  double Irms;
  
//setup RTC module. DS3231 or DS1307. Uses I2C communciation
  /* SDA to Arduino A4
     SCL to Arduino A5
     */
  #include "RTClib.h"
   //  RTC_DS3231 rtc; //if using DS3231 RTC module
  RTC_DS1307 rtc; //if using DS1307 RTC module
  int oldMinute; //for triggering readings every minute
  DateTime now;


//Set up Wifi
  #include <SoftwareSerial.h>
  #include <stdlib.h>
  #define DEBUG 1   // change value to 1 to enable debuging using serial monitor 
  String network = "Natashaphone" ;    // "SSID-WiFiname"
  String password = "smartvils"; // "password"
  #define IP "184.106.153.149"      // thingspeak.com ip
  String GET = "GET /update?key=F9Q1ROVIZW8TLDYE"; //change it with your api key like "GET /update?key=Your Api Key"
  
  //Hardware serial Rx and Tx pins on Arduino (0,1) are needed for serial communication.
  //Use SoftwareSerial to enable Rx, Tx comms alongside Hardware Serial pins.
  SoftwareSerial esp8266Module(2,3);  //(Rx, Tx)



void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  SDCardInitialise();
  SDCardPrintHeadings();

  setUpRTC();
  setUpACCurrentSense();
  
  esp8266Module.begin(9600);  //start communication with wifi module
  delay(4000);
}


void loop() {

  //record current time
  now = rtc.now();

  if (now.minute()!=oldMinute){ //Log data every minute
    oldMinute= now.minute();
    
    //get Voltage reading and Current readings
    dcVoltage_average = getVoltageAverage();
    dcCurrent_average = getDCCurrentAverage();
    Irms = emon1.calcIrms(1480);
    apparentPow= Irms*(float)ACVoltage;
  
    //Any logic to apply switching signal to operate dump load
    //e.g. if (dcCurrent_average > currentThreshold && chargeController ==float mode && dumpstatus ==False && TimeTriggered-now.minute() <10){
    //        TriggerDumpLoad()
    //        dumpstatus==True
  //          if dumpstatus==True{
  //            dcCurrentold = dCurrent_average
  //            if dcCurrent_average << dcCurrentOld{
  //              TurnOffDumpLoad()
  //              dumpStatus==False
  //              TimeTriggered = now.minute()
  //            }
  //          }
  
  
  //Write data and timestamp to SD card
  writeDataToSDCard();
  
//  setupEsp8266();
  updateThingSpeak(String(dcVoltage_average),String(dcCurrent_average),String(Irms),String(apparentPow));
  }
}



void readDCCurrent(){
    int A1Value = analogRead(dcCurrentAnalogPin);
    float dcCurrent_sensed = A1Value * (arduinoVCC / (float)inputResolution); 
//  Serial.print("dcCurrent_sensed:");
//  Serial.print(dcCurrent_sensed);       
  dcCurrent = dcCurrent_sensed * (float)CurrentSensorRating / arduinoVCC;
}


float getDCCurrentAverage(){
  float dcCurrent_average =0;
  for(int i=0; i < average_of; i++)
    {
       readDCCurrent();
       dcCurrent_average +=dcCurrent;
    }
  return dcCurrent_average / average_of;
}

void readVoltage(){
    int A0Value = analogRead(voltageAnalogPin);
    float voltage_sensed = A0Value * (arduinoVCC / (float)inputResolution); 
//  Serial.print("voltage_sensed:");
//  Serial.print(voltage_sensed);       
    voltage = voltage_sensed * ( 1 + ( (float) ValueR2 /  (float) ValueR1) );
}

float getVoltageAverage(){

    float voltage_temp_average=0;
    for(int i=0; i < average_of; i++)
    {
       readVoltage();
       voltage_temp_average +=voltage;
    }     
  return voltage_temp_average / average_of;
}


void SDCardInitialise(){
  
  Serial.print(F("Initializing SD card..."));

  //check card is present and working
  if (!SD.begin(chipSelect)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("Initialization done."));
    
  //Set up SD card for data logging
  //Data logging done in files with a name like DATAXX.CSV where XX is a number from 00 to 99.
  //Check the SD card for existing file names, and pick the latest unused file name. e.g. DATA15.CSV
     // create a new file
     for (uint8_t i = 0; i < 100; i++) {
        filename[4] = i/10 + '0';
        filename[5] = i%10 + '0';
        if (! SD.exists(filename)) {
           Serial.print(F("Data file is ")); Serial.println(filename);
           // only open a new file if it doesn't exist
           break;  // leave the loop! filename will now be the one we desire
        }      
     }
}

void  SDCardPrintHeadings(){

    dataFile = SD.open(filename, FILE_WRITE);      
    if (dataFile) {
        dataFile.print("Date (DD/MM/YYYY)");
        dataFile.print(",");
        dataFile.print("Time");
        dataFile.print(",");
        dataFile.print("DC Voltage");
        dataFile.print(",");
        dataFile.print("DC Current");
        dataFile.print(",");
        dataFile.print("AC Current");
        dataFile.print(",");
        dataFile.println("Apparent Power");
        dataFile.close();
    }
    //if the file isn't open, pop up an error:
    else {
        Serial.print(F("error printing headings to ")); Serial.println(filename);
      } 
    //print to the serial port too:
//    Serial.print(F("Date (DD/MM/YYYY)"));
//    Serial.print(F(","));
//    Serial.print(F("Time"));
//    Serial.print(F(", "));
//    Serial.print(F("DC Voltage"));
//    Serial.print(F(", "));
//    Serial.print(F("DC Current"));
//    Serial.print(F(", "));
//    Serial.print(F("AC Current"));
//    Serial.print(F(", "));
//    Serial.println(F("Apparent Power"));
//
}

void setUpRTC(){
  
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println(F("RTC is NOT running, let's set the time!"));
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

void  writeDataToSDCard(){

  dataFile = SD.open(filename, FILE_WRITE);      
  if (dataFile) {
      dataFile.print(now.day(), DEC);
      dataFile.print('/');
      dataFile.print(now.month(), DEC);
      dataFile.print('/');
      dataFile.print(now.year(), DEC);
      dataFile.print(" ");
      dataFile.print(now.hour(), DEC);
      dataFile.print(':');
      dataFile.print(now.minute(), DEC);
      dataFile.print(':');
      dataFile.print(now.second(), DEC);
      dataFile.print(",");
      
      //printing actual data
      dataFile.print(",");
      dataFile.print(dcVoltage_average);
      dataFile.print(",");
      dataFile.print(dcCurrent_average);
      dataFile.print(",");        
      dataFile.print(Irms);
      dataFile.print(","); 
      dataFile.println(apparentPow);
      dataFile.close();
  }
  //if the file isn't open, pop up an error:
  else {
      Serial.print(F("error opening ")); Serial.println(filename);
    }
  
  //print to the serial port too:
//      Serial.print(now.day(), DEC);
//      Serial.print(F("/"));
//      Serial.print(now.month(), DEC);
//      Serial.print(F("/"));
//      Serial.print(now.year(), DEC);
//      Serial.print(F(" "));
//      Serial.print(now.hour(), DEC);
//      Serial.print(F(":"));
//      Serial.print(now.minute(), DEC);
//      Serial.print(F(":"));
//      Serial.print(now.second(), DEC);
//      Serial.print(F(","));
//      Serial.print(dcVoltage_average);
//      Serial.print(F(","));
//      Serial.print(dcCurrent_average);
//      Serial.print(F(","));        
//      Serial.print(Irms);
//      Serial.print(F(","));    
//      Serial.println(apparentPow);
}


void setUpACCurrentSense(){
  emon1.current(ACCurrentAnalogPin, ACSensorCalibration);
}

//void setupEsp8266()                                   
//{
//    if(DEBUG){
//      Serial.println(F("Reseting esp8266"));
//    }
//    esp8266Module.flush();
//    esp8266Module.println(F("AT+RST"));
//
//    //PrintStringFromEspModule(); Useful for debugging but note .find("OK") won't work.  
//      
//    delay(7000);
//    //Serial.println(F("Finished reseting"));
//    if (esp8266Module.find("OK"))
//    {
//      if(DEBUG){
//        //Serial.println(F("Found OK"));
//        //Serial.println(F("Changing espmode"));
//      }  
//      esp8266Module.flush();
//      changingMode();
//      esp8266Module.flush();
//      connectToWiFi();
//    }
//    else
//    {
//      if(DEBUG){
//        Serial.println(F("OK not found"));
//      }
//    }
//}

//-------------------------------------------------------------------
// Following function sets esp8266 to station mode
//-------------------------------------------------------------------
//bool changingMode()
//{
//    esp8266Module.println(F("AT+CWMODE=1"));
//    delay(5000);
//    if (esp8266Module.find("OK"))
//    {
//      if(DEBUG){
//        Serial.println(F("Mode changed"));
//      }  
//      return true;
//    }
//    else
//    {
//      if(DEBUG){
//        Serial.println(F("Error while changing mode"));
//      }  
//      return false;
//    }
//}

//-------------------------------------------------------------------
// Following function connects esp8266 to wifi access point
//-------------------------------------------------------------------
//bool connectToWiFi()
//{
//  if(DEBUG){
//    //Serial.println(F("inside connectToWiFi"));
//  }  
//  String cmd = F("AT+CWJAP=\"");
//  cmd += network;
//  cmd += F("\",\"");
//  cmd += password;
//  cmd += F("\"");
//  esp8266Module.println(cmd);
//  //PrintStringFromEspModule();  //useful for debugging, but note esp8266Module.find("OK") won't work if it is read and printed here first.
//  delay(15000);
//  if (esp8266Module.find("OK"))
//  {
//    if(DEBUG){
//      Serial.println(F("Connected to Access Point"));
//    }  
//    return true;
//  }
//  else
//  {
//    if(DEBUG){
//      Serial.println(F("Could not connect to Access Point"));
//    }  
//    return false;
//  }
//}

//-------------------------------------------------------------------
// Following function sends sensor data to thingspeak.com
//-------------------------------------------------------------------
void updateThingSpeak(String value1,String value2, String value3, String value4)
{  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  esp8266Module.println(cmd);
  delay(5000);
  if(esp8266Module.find("Error")){
    if(DEBUG){
      Serial.println(F("ERROR while SENDING"));
    }  
    return;
  }
  cmd = GET + "&field1=" + value1 + "&field2=" + value2 + "&field3=" + value3 + "&field4=" + value4 + "\r\n";
  //cmd = GET + "&field1=" + value1 + "&field2=" + value2 + "\r\n";
  esp8266Module.print("AT+CIPSEND=");
  esp8266Module.println(cmd.length());
  delay(15000);
  if(esp8266Module.find(">"))
  {
    esp8266Module.print(cmd);
    if(DEBUG){
      Serial.println(F("Data sent"));
    }
  }
  else
  {
    esp8266Module.println("AT+CIPCLOSE");
    if(DEBUG){
      Serial.println(F("Connection closed"));
    }  
  }
}


//void PrintStringFromEspModule()
////useful for debugging. Serial Print everything that ESP module sends in response.
//{
//  long int time = millis();                                         //get the operating time at this specific moment and save it inside the "time" variable.      
//  String response = "";   
//  while( (time+3000) > millis())                                 //excute only whitin 1 second.      
//  {            
//    while(esp8266Module.available())                                      //is there any response came from the ESP8266 and saved in the Arduino input buffer?      
//    {      
//       char c = esp8266Module.read();                                      //if yes, read the next character from the input buffer and save it in the "response" String variable.      
//       response+=c;                                                  //append the next character to the response variabl. at the end we will get a string(array of characters) contains the response.      
//    }        
//  }          
//  if(DEBUG)                                                         //if the "debug" variable value is TRUE, print the response on the Serial monitor.      
//  {      
//    Serial.print(response);      
//   } 
//}         
