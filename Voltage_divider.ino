const float arduinoVCC = 5.01;//Your Arduino voltage
const int inputResolution =1023;//works with most Arduino boards

unsigned long ValueR1 = 10000;
unsigned long ValueR2 = 1000;
const int voltageAnalogPin = A0;//the pin connecting the voltage. 
const float average_of = 500;//Average of 500 readings

float voltage;
void setup() {
  Serial.begin(9600);
  Serial.println("Reading voltage with Arduino");
  delay(500);

}

void loop() {
  
  //readVoltage(); 
  //Serial.print("Vin: ");
  //Serial.print(voltage);
  Serial.print("V Avg: ");
  Serial.print(getVoltageAverage());
  Serial.println("V");
  //delay(100);


}//loop end


/*
 * @brief calculates the input voltage and updates the variable "voltage"
 * @param none
 * @return does not return anything
 */
void readVoltage(){
    int A0Value = analogRead(voltageAnalogPin);
    float voltage_sensed = A0Value * (arduinoVCC / (float)inputResolution); 
//  Serial.print("voltage_sensed:");
//  Serial.print(voltage_sensed);       
  voltage = voltage_sensed * ( 1 + ( (float) ValueR2 /  (float) ValueR1) );
  
}

/*
 * @brief calculates the average of input voltage and updates the variable "voltage"
 * @param none
 * @return retuns average of "average_of" iteration of voltage
 */
float getVoltageAverage(){

    float voltage_temp_average=0;
    for(int i=0; i < average_of; i++)
    {
       readVoltage();
       voltage_temp_average +=voltage;
    }
      
  return voltage_temp_average / average_of;
}
