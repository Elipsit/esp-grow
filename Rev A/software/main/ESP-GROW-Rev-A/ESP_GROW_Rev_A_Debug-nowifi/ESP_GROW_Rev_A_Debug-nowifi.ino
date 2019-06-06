/* ___________________________________
 |   ESP-Grow Rev A - Debug No Wifi   |
 |   By Kyle Rodrigues                |
 |   Date: 6/3/2019                   |
  ___________________________________

***Library Versions:***
ESP Version: 2.5.0
Board: Wemos D1 R1
DHT Sensor Library: 1.3.4
Adafruit Sensor: 
Adafruit SSD1306:

***Notes***
-Use the GPIO# pins for mapping.
-Make sure to add these:
http://arduino.esp8266.com/stable/package_esp8266com_index.json

***Pin Assignment***   
DHT22 - GPIO2 (D4)
OLED Screen -GPIO4, GPIO5, Addr 0x3C, character uses 5x5 pixels, (X,Y)
soil capacitive sensor - A0
Water Pump - GPIO16
Pump Button - GPIO12
Screen Button - GPIO0

***Change Log***
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

int  soilana,soilval;
float t,h;
const int pump = 16; // water pump
const int btn = 12; //push button for pump
const int oled_btn = 0; //OLED Screen button


void setup(void) {
Serial.begin(115200); //changed from 115200
 pinMode(pump, OUTPUT);
 pinMode(btn, INPUT);
  pinMode(oled_btn, INPUT);

 
    //******OLED Start*****
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
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
    display.println("ESP-Grow Debug");
    display.display();       // Refresh the display
    delay(100);          // Delay a second and repeat 
}

void loop() {
  soilana = analogRead(A0);
  soilval =  map(soilana,381,836,100,0);
  h = dht.readHumidity();
  t = dht.readTemperature();
  display.clearDisplay();
  display.setCursor(10, 0); // Set cursor to top-left
  display.println("ESP-Grow Debug");
  display.setCursor(37, 0);
  display.println("A0:");
    display.setCursor(37, 20);
  display.println(soilana);
  
   //Soil Capacitance Sensor
  display.setCursor(0, 30);
   if(soilval < 5){ 
     display.println("No Soil Measured"); //really dry soil or not inserted correctly 
   }
   else if(soilval > 110){
     display.println("Soil Sens Error");    //no soil sensor detected
   }
   else{
  display.println("Soil:");  
  display.setCursor(35, 30);
  display.println(soilval);  
  display.setCursor(50, 30);
  display.println("%");
   }
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
  delay(100);
  display.display();
  if(soilval<40){
    waterpump();
  }
  server.handleClient();
}

  void waterpump(){
    digitalWrite(pump, HIGH);
    delay(5000);
    digitalWrite(pump, LOW);
  }
