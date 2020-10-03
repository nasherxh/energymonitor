const float arduinoVCC = 5.01; //Arduino Voltage
const int inputResolution = 1023;

const int dcCurrentAnalogPin = A1;//the pin connecting the voltage.
const float average_of = 500;
const int CurrentSensorRating = 100;
float dcCurrent;

 void setup() {
  Serial.begin(9600);
  Serial.println("Reading DC current with Arduino");
  delay(500);

}

void loop() {
  Serial.print("Current Avg: ");
  Serial.print(getDCCurrentAverage());
  Serial.println("A");

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
