/*ESP-Grow - No WIFI
By Kyle Rodrigues
Date: 03032019

**Note: This code works, but soil refresh rate is buggy. need to edit logic flow

DHT22 - D4 pin 2
Screen - OLED Screen , Addr 0x3C, character uses 5x5 pixels, (X,Y)
soil capacitive sensor A0
*/

//****Include Statements*****
//OLED Screen
#include <Wire.h>  // Include Wire if you're using I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//DHT22
#include "DHT.h"


//****Definitions*****
//*****OLED 0.96" **********
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     1 // // Wemo SCL 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//*****DHT22 **********
#define DHTPIN 2     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
DHT dht(DHTPIN, DHTTYPE);

int  soilval;
float t,h;


void setup() {
Serial.begin(115200);
 
    //******OLED Start*****
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
    display.display();
    delay(2000); // Pause for 2 seconds
    // Clear the buffer
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(10, 0); // Set cursor to top-left
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // Print can be used to print a string to the screen:
    display.println("Girl Scout Cookies");
    display.display();       // Refresh the display
    delay(100);          // Delay a second and repeat
}

void loop() {
  soilval =  map(analogRead(0),320,750,100,0);
  h = dht.readHumidity();
  t = dht.readTemperature();
  display.clearDisplay();
  
  display.setCursor(10, 0); // Set cursor to top-left
  display.println("Girl Scout Cookies");
   //Soil Capacitance Sensor 
  display.setCursor(0, 30);
  display.println("Soil:");  
  display.setCursor(35, 30);
  display.println(soilval);  
  display.setCursor(50, 30);
  display.println("%");
  //DHT22 Temperature and Humidity
  display.setCursor(0, 40);
  display.println("DHT22:");
  display.setCursor(35, 40);
  display.println(dht.readTemperature());
  display.setCursor(70, 40);
  display.println("oC");
  display.setCursor(35, 50);
  display.println(dht.readHumidity());
  display.setCursor(70, 50);
  display.println("%");  
  display.display();
  delay(1000);
  display.display();
  }

