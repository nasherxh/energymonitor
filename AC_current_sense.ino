#include "EmonLib.h"


//set up AC current sensor
EnergyMonitor emon1;
const int ACCurrentAnalogPin = 2;
const int ACSensorCalibration = 30; // Current want to read when 1V produced
const int ACVoltage = 230.0; //rms voltage of thing sensing
double Irms;

void setup()
{
  Serial.begin(9600);

  emon1.current(ACCurrentAnalogPin, ACSensorCalibration);             
}

void loop()
{
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
  Serial.print(Irms*(float)ACVoltage);           // Apparent power
  Serial.print(",");
  Serial.println(Irms);             // Irms
}
