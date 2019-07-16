/* ___________________________________
 |   ESP-Grow Rev A                   |
 |   By Kyle Rodrigues                |
 |   Date: 7/15/2019                  |
  ___________________________________

NOTE: CODED FOR PLATFORM IO NOT ARDUINO IDE

***Library Versions:***
ESP Version: 2.5.0
Board: Wemos D1 R1
DHT Sensor Library: 1.3.4


***Pin Assignment***   
DHT22 - GPIO2 (D4)
OLED Screen -GPIO4, GPIO5, Addr 0x3C, character uses 5x5 pixels, (X,Y)
soil capacitive sensor - A0
Water Pump - GPIO16
Pump Button - GPIO12
Screen Button - GPIO0
*/

#include <Arduino.h>
#include "DHTesp.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

  // Autodetect is not working reliable, don't use the following line
  //dht.setup(2);
  // use this instead: 
  dht.setup(2, DHTesp::DHT11); // Connect DHT sensor to GPIO 2
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  delay(2000);
}