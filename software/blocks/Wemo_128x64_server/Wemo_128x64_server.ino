/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "Pretty Fly For A Wifi-2.4";
const char* password = "supercarbon";

ESP8266WebServer server(80);

#define OLED_RESET 1

//Variables

const int buttonPin = 3;
int buttonState = 0;         // variable for reading the pushbutton status

Adafruit_SSD1306 display(OLED_RESET);

void handleRoot() {
 
  server.send(200, "text/plain", "hello from esp8266!");

}




void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup()   {                
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.display();
  delay(1000);

pinMode(buttonPin, INPUT);

  // Clear the buffer.
  display.clearDisplay();

  display.display();
  delay(2000);
  display.clearDisplay();
  
 Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
   display.setTextSize(0);
  display.setTextColor(WHITE);

 // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  
   display.clearDisplay();
  display.setCursor(30,0);
  // Print can be used to print a string to the screen:
  display.println("Wifi Button");
  display.display();       // Refresh the display
  delay(1000);          // Delay a second and repeat
 display.setCursor(30,20);
  display.println("Connecting...");
  display.display();       // Refresh the display
}

 Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  display.clearDisplay();
  display.setCursor(30,0);
  display.println("Wifi Button");
  display.setCursor(30,20);
  display.println(WiFi.localIP());
   display.display();       // Refresh the display
   delay(1000);

    if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  buttonState = digitalRead(buttonPin);
if (buttonState == HIGH) {     

  
  display.setCursor(30,10); 
  display.println("HIGH");
  } 
  else {
    // turn LED off:
     display.setCursor(30,10); 
  display.println("LOW"); 
  }
  delay(1000);
}

