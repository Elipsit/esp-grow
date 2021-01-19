#include <Arduino.h>

float calibration = 21.70; //change this value to calibrate
const int analogInPin = 25; 
int sensorValue = 0; 
unsigned long int avgValue; 
float b;
int buf[10],temp;

void setup() {
 Serial.begin(115200);
}
 
void loop() {
 for(int i=0;i<10;i++) 
 { 
 buf[i]=analogRead(analogInPin);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buf[i]>buf[j])
 {
 temp=buf[i];
 buf[i]=buf[j];
 buf[j]=temp;
 }
 }
 }
 avgValue=0;
 for(int i=2;i<8;i++)
 avgValue+=buf[i];
 float pHVol=(float)avgValue*7.35/4095/6; //Vcc + 2.5V
 float phValue = -5.70 * pHVol + calibration;
 Serial.print("pH Value= ");
 Serial.println(phValue);
 
 delay(500);
}
