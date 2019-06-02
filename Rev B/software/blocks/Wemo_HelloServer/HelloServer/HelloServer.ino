#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>  // Include Wire if you're using I2C
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

const char* ssid = "Pretty Fly For A Wifi-2.4";
const char* password = "supercarbon";

ESP8266WebServer server(80);

#define PIN_RESET 1  // Wemo SCL 
MicroOLED oled(PIN_RESET, 0x3C);    // I2C declaration


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

void setup(void) {
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  delay(1000);     // Delay 1000 ms
  oled.clear(PAGE); // Clear the buffer.
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    oled.setFontType(0);  // Set font to type 1
  oled.clear(PAGE);     // Clear the page
  oled.setCursor(8, 0); // Set cursor to top-left
  // Print can be used to print a string to the screen:
  oled.print("Wifi Test");
  oled.display();       // Refresh the display
  delay(1000);          // Delay a second and repeat
    oled.setCursor(0, 20);
  oled.print("Get WIFI");
  oled.display();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

    oled.setCursor(0, 20);
  oled.print(WiFi.localIP());
  oled.display();
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
	moist();
  server.handleClient();
  delay(1000);
  
}


void moist() {
	int mositsens = analogRead(0);
	Serial.println(mositsens);
	oled.setCursor(0, 40);
	oled.print(mositsens);
	oled.display();
}