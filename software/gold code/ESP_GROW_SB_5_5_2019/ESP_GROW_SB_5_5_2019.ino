/*ESP-Grow - No WIFI
By Kyle Rodrigues
Date: 04/07/2019

Board: Wemos D1 esp8266

Make sure to add these:
   https://dl.espressif.com/dl/package_esp32_index.json,
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   
   DHT22 - D4 pin 2
Screen - OLED Screen , Addr 0x3C, character uses 5x5 pixels, (X,Y)
soil capacitive sensor A0

Changes:
-5/5/2019 - added plant description
-Soil Sens Error Added; no soil detected and no sensor detected
-adding variable for button 
-water pump added; set to water for 10 sec when soil is < 50% moisture
*/

//****Include Statements*****
//OLED Screen
#include <Wire.h>  // Include Wire if you're using I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//DHT22
#include "DHT.h"
//ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

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
const int pump = 16; // water pump
const int btn = 15; //push button

// Replace with your network credentials
const char* ssid = "Pretty Fly For A Wifi-2.4";
const char* password = "supercarbon";
 
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
 
String page = "";
double data; 

void setup(void) {
Serial.begin(115200);
 pinMode(pump, OUTPUT);
 pinMode(btn, INPUT);

//Revision
 Serial.println(F("ESP-GROW_04072019"));
 
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
    display.println("Strawberry Banana");
    display.display();       // Refresh the display
    delay(100);          // Delay a second and repeat

    //******ESP8266 Startup Code******
     WiFi.begin(ssid, password); //begin WiFi connection
     Serial.println("");
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      display.setCursor(40, 20);
      display.println("Get WIFI");
      display.display();
      delay(100);
      display.display();
      }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.display();
    delay(100);
    display.setCursor(10, 0); // Set cursor to top-left
    display.println("Strawberry Banana");
    display.setCursor(35, 15);
    display.println(WiFi.localIP());
    display.display();
    delay(100);
    display.display();
    server.on("/", [](){
    page = "<h1>ESP-GROW Strawberry Banana</h1></h2>By: Kyle Rodrigues | 05/05/2019</h2></p>Strawberry Banana is a 70/30 indica-dominant cannabis strain developed by DNA Genetics in collaboration with Serious Seeds. A genetic cross between Crocketts Banana Kush and the *Strawberry* phenotype of Bubble Gum, Strawberry Banana inherits a sweet fruity flavor that earned this hybrid her name. Known for its heavy resin production and high THC content, Strawberry Banana produces happy, peaceful effects that sharpen creativity and sensory awareness.</p><h3>Soil Moisture Content(%):</h3> <h4>"+String(soilval)+"</h4><h3>Temperature(oC):</h3> <h4>"+String(t)+"</h4><h3>Humidity(%):</h3> <h4>"+String(h)+"</h4>";
    server.send(200, "text/html", page);
  });
  
  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  soilval =  map(analogRead(0),320,760,100,0);
  h = dht.readHumidity();
  t = dht.readTemperature();
  display.clearDisplay();
  display.setCursor(10, 0); // Set cursor to top-left
  display.println("Strawberry Banana");
  display.setCursor(37, 20);
  display.println(WiFi.localIP());
  
   //Soil Capacitance Sensor
  display.setCursor(0, 30);
   if(soilval < 5){ 
     display.println("No Soil Measured"); //really dry soil or not inserted correctly 
   }
   else if(soilval > 100){
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
  if(soilval<60){
    waterpump();
  }
  server.handleClient();
}

  void waterpump(){
    digitalWrite(pump, HIGH);
    delay(10000);
    digitalWrite(pump, LOW);
  }

