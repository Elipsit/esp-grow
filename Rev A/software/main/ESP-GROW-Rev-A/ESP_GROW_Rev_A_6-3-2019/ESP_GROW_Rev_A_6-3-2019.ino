/* ___________________________________
 |   ESP-Grow Rev A                   |
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
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

int  soilana,soilval;
float t,h;
const int pump = 16; // water pump
const int btn = 12; //push button for pump
const int oled_btn = 0; //OLED Screen button
char* product = "Blueberry Dream";
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];


//ESP8266 Setup
char* ssid = "Pretty Fly For A Wifi-2.4";
char* password = "supercarbon";

WiFiServer server(80); //instantiate server at port 80 (http port)

// Client variables
char linebuf[80];
int charcount = 0;

void setup() {
 // initialize the DHT sensor
  dht.begin();
// initialize buttons:
 pinMode(pump, OUTPUT);
 pinMode(btn, INPUT);  

  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

 
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
    display.println(product);
    display.display();       // Refresh the display
    delay(100);          // Delay a second and repeat

 //******ESP8266 Startup Code******
 // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    delay(500);
    Serial.print(".");
    display.setCursor(40, 20);
    display.println("Get WIFI");
    display.display();
    delay(100);
    display.display();
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
      display.clearDisplay();
    display.display();
    delay(100);
    display.setCursor(10, 0); // Set cursor to top-left
    display.println(product);
    display.setCursor(37, 15);
    display.println(WiFi.localIP());
    display.display();
    delay(100);
    display.display();

  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    memset(linebuf, 0, sizeof(linebuf));
    charcount = 0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        //read char by char HTTP request
        linebuf[charcount] = c;
        if (charcount < sizeof(linebuf) - 1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
            display.clearDisplay();
            display.setCursor(10, 0); // Set cursor to top-left
            display.println(product);
            display.setCursor(37, 20);
            display.println(WiFi.localIP());

            soilval =  map(analogRead(0),320,760,100,0);
            
            // DHT Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
            h = dht.readHumidity();
            t = dht.readTemperature();   
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
                  
            // Check if any reads failed and exit early (to try again).
          if (isnan(h) || isnan(t)) {
            Serial.println("Failed to read from DHT sensor!");
            strcpy(celsiusTemp, "Failed");
            strcpy(fahrenheitTemp, "Failed");
            strcpy(humidityTemp, "Failed");
          }
          else {
            // Computes temperature values in Celsius + Fahrenheit and Humidity
            float hic = dht.computeHeatIndex(t, h, false);
            dtostrf(hic, 6, 2, celsiusTemp);
            dtostrf(h, 6, 2, humidityTemp);
            
          }
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
            
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<meta http-equiv=\"refresh\" content=\"30\"></head>");
          client.println("<body><div style=\"font-size: 1.5rem;\"><h1>ESP-GROW Web Server</h1></h2>By: Kyle Rodrigues | 06/03/2019</h2><p>");
          client.println(product);
          client.println("<div style=\"color: #484D79;\"></h2>Air Temperature:</h2><p>");
          if (atoi(celsiusTemp) >= 30) {
            client.println("<div style=\"color: #930000;\">");
          }
          else if (atoi(celsiusTemp) < 20 && atoi(celsiusTemp) >= 5) {
            client.println("<div style=\"color: #006601;\">");
          }
          else if (atoi(celsiusTemp) < 5) {
            client.println("<div style=\"color: #009191;\">");
          }
          client.println(celsiusTemp);
          client.println("*C</p><p>");
          client.println("<div style=\"color: #484D79;\"></h2>Humidity:</h2><p>");
         client.println("<div style=\"color: #006601;\">");
          client.println(humidityTemp);
          client.println("%</p><p>");
          client.println("<div style=\"color: #484D79;\"></h2>Soil Moisture Content:</h2><p>");
         client.println("<div style=\"color: #006601;\">");
          client.println(soilval);
          client.println("%</p></div>");
          client.println("</body></html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf, 0, sizeof(linebuf));
          charcount = 0;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

  void waterpump(){
    digitalWrite(pump, HIGH);
    delay(10000);
    digitalWrite(pump, LOW);
  }
