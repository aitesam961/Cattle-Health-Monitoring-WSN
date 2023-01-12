#include <Arduino.h>    //Header file for LCD
const int rs=12, en=11, d4=5, d5=4, d6=3, d7=2; //pins of LCD connected to Arduino


const int aqsensor = 35;  //output of mq135 connected to A0 pin of Arduino
int threshold = 250;      //Threshold level for Air Quality

void setup() {
     // led is connected as output from Arduino
  pinMode (aqsensor,INPUT); // MQ135 is connected as INPUT to arduino

  Serial.begin (115200);      //begin serial communication with baud rate of 9600
}

void loop() {

  int ppm = analogRead(aqsensor); //read MQ135 analog outputs at A0 and store it in ppm

  Serial.print("Air Quality: ");  //print message in serail monitor
  Serial.println(ppm);            //print value of ppm in serial monitor

  if (ppm > threshold)            // check is ppm is greater than threshold or not
    { 
      Serial.println("AQ Level HIGH");
    }
  else
    {
      Serial.println("AQ Level Good");
    }  
  delay (500);
}