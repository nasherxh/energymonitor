# energymonitor
DC current, DC voltage and AC current sensor. Writes to SD card and thingspeak.

Lots of stuff that needs fixing/improving with arduino sensor code.

ESP32 is an alternative microcontroller with inbuilt wifi. Should do a brief bit of research into this if memory continues to be an issue with the arduino, as it could be a better alternative, and can be programmed with arduino IDE. Would need to check GPIO for ESP32 and if it has sufficient analogue inputs etc, and these pins would need changing in the code, but general programming for sensing, SD card writing and RTC should remain the same. Would need to use slightly different sensors though as ESP32 operates pins at 3.3V, not 5V logic like Arduino (I've got these already so can test it). Wifi should be simpler to troubleshoot and wouldn't need external module, so this may be a better option.

AC current sense :: basic code for using emonlib to use AC current sensor

DC_Sense_and_Write:  This is the main code which attempts to do everything.
                  Currently sections are commented out as it can't write to SD card & to cloud simultaneously. It runs out of dynamic memory.
                  
DC_current_sense: For testing reading DC current only. 

RTC_Test: For testing real time clock module

SD_Card_Test: To check SD card works

Voltage_divider: Testing DC voltage divider only. No writing to SD card or cloud.

WifiTest: Test writing with ESP8266 to cloud

wifiTest2: allows more debugging where create a pass-through serial connection to the ESP module to program it directly, and serial print the repsonse.



To do:

-Edit code so that can write to cloud and SD card simultaneously (currently runs out of dynamic memory if try to do both, then neither work)

-Check that calculations for DC current/voltage aren't poorly written, resulting in rounding errors (from float/int stuff?). If this is fine, then either sensors are faulty or there is a calibration issue, as the DC current/voltage tests gave the wrong values when being tested. Current showed 9.88A when should be 0, and 20A when should be 14A. Voltage showed 0.56V when should be 0V, then 4.88V when should be 206V (wayy off)

-Find out how much data can be saved to microusb if measure every minute. (I read somewhere that even if SD card has 32GB memory, the arduino can only recognise a small fraction of that space (maybe 2GB?? need to check)

-Thoughts about saving data if didn't manage to send to cloud, and then send it at a later point? Currently just sends data to thingspeak in real time, and thingspeak gives it a timestamp. The arduino is measuring the time using the RTC module anyway to write to SD card, so could also send this data.

-Understand what Bernie actually wants this to usefully do with the data read (something to do with immersion heater and charge controller logic?). If there are significant calculations required, then this is going to add more memory issues.

-Find out about energy saving and if should make Arduino enter power saving mode between readings. (Is definitely something that ESP32 does)

-OPTIONAL/FALL BACK: Work out PinIn/Out wiring diagram for ESP32, to determine any additional components required. (optional if Arduino proves not suitable, or think that inbuilt wifi on ESP32 would be much better)
