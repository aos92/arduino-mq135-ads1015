
#include <Wire.h>
#include "Adafruit_ADS1015.h"

Adafruit_ADS1015 adc;
unsigned int adc0 = 0;

unsigned int values[25];
unsigned int num_values = 25;
unsigned int sum = 0;
unsigned int i = 0;

void setup(){
  Serial.begin(9600);
  //pinMode(12, OUTPUT);
  adc.begin();
  adc.setGain(GAIN_SIXTEEN); 
  //analogReference(EXTERNAL); //reference high adc voltage as AREF
    
}

void loop(){  
  i = 0;
  sum = 0;
  
  while (i < num_values){
     values[i++] = adc0 = adc.readADC_SingleEnded(0);
     delay(10);  
  }
  
  i = 0;
  
  while(i<num_values) {
    sum += values[i++];
  }
  
  sum /= num_values;
  
  Serial.print("ADC reading is: ");
  Serial.print(sum);
  Serial.println("Z");
  delay(100);
}
