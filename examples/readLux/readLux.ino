/*****************************************************************************************************
File:             readLux.ino
Description:      This example demonstrates how to initialize the VEML7700 and then get the ambient light lux.
Note:             
******************************************************************************************************/
#include "BME82M131.h"

BME82M131 ALS;           //Create a BME82M131 object

void setup()
{
  Serial.begin(9600);
  ALS.begin();
  Serial.print(ALS.getNumber());
  Serial.print(" modules are ");
  Serial.println("Connected!");
  Serial.println("modle is Connected!");
}
void loop()
{
  Serial.print("Lux1:");
  Serial.println( ALS.readLux(1));     // Read the lux from the sensor1 and print it
  delay(1000);
  
}