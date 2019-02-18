/*ESP-Grow
By Kyle Rodrigues
Date: 02172019

DHT22 - D4
Screen - micro OLED - normal works too
soil capacitive sensor A0

Changes: title change to esp-grow
-oled delays changed from 1000 to 100
-DHT if statement >1000 from >2000
-map function applied to moisture
*/

//****Include Statements******
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Wire.h>  // Include Wire if you're using I2C
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

#include "DHT.h"
#define DHTPIN 2     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
DHT dht(DHTPIN, DHTTYPE);
int timeSinceLastRead = 0;

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
	Serial.begin(115200);
	Serial.println("Running OLED");
	oled.begin();    // Initialize the OLED
	oled.clear(ALL); // Clear the display's internal memory
	oled.display();  // Display what's in the buffer (splashscreen)
	delay(1000);     // Delay 1000 ms
	oled.clear(PAGE); // Clear the buffer.

	Serial.println("Running DHT!");

	Serial.println("Running Wifi!");
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
		oled.print("ESP-Grow");
		oled.display();       // Refresh the display
		delay(100);          // Delay a second and repeat
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
	delay(100);

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
	DHT22Sens();
	server.handleClient();
	delay(100);

}


void moist() {
	int mositsens = map(analogRead(0),320,750,100,0);
	Serial.println(mositsens);
	oled.setCursor(0, 40);
	oled.print(mositsens);  
  oled.setCursor(15, 40);
  oled.print("%");
	oled.display();
	delay(100);
	timeSinceLastRead += 100;
}

void DHT22Sens() {

	if (timeSinceLastRead > 1000) {
		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		float h = dht.readHumidity();
		// Read temperature as Celsius (the default)
		float t = dht.readTemperature();

		// Check if any reads failed and exit early (to try again).
		if (isnan(h) || isnan(t)) {
			Serial.println("Failed to read from DHT sensor!");
			timeSinceLastRead = 0;
			return;
		}
		float hic = dht.computeHeatIndex(t, h, false);

		Serial.print("Humidity: ");
		Serial.print(h);
		Serial.print(" %\t");
		Serial.print("Temperature: ");
		Serial.print(t);
		Serial.print(" *C ");
		Serial.print("Heat index: ");
		Serial.print(hic);
		Serial.print(" *C ");

		oled.setCursor(30, 40);
		oled.print(t);
    oled.setCursor(55, 40);
    oled.print("C");
		oled.display();
		delay(100);          // Delay a second and repeat
		oled.display();

		timeSinceLastRead = 0;
	}
}
