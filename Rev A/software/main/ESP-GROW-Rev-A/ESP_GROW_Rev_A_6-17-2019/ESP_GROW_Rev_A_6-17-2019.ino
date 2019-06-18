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

Server Sample Code:
© Copyright 2017, Ivan Grokhotkov Revision ec7bdfa2.
https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/server-examples.html

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
*6/17/2019
*new webpage reference code; easier to use
*changed water timer delay to 5 sec
*added a short delay to use when pump button pressed
*added an uptime counter
*
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
const int btn = 12; //push button
const int scrbtn = 0; //push button

//***Uptime deff***
long Day=0;
int Hour =0;
int Minute=0;
int Second=0;
int HighMillis=0;
int Rollover=0;

char* product = "Strawberry Banana";
char* revision = "Code Version: 6/17/2019";

const char* ssid = "Pretty Fly For A Wifi-2.4";
const char* password = "supercarbon";

WiFiServer server(80);

void setup(){
  Serial.begin(115200);
  Serial.println();
  
  //Revision
 Serial.println(revision);
 
 //GPIO
   pinMode(pump, OUTPUT);
 pinMode(btn, INPUT);
  pinMode(scrbtn, INPUT);

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
  
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    display.setCursor(40, 20);
    display.println("No WIFI");
    display.display();
    delay(100);
    display.display();
  }
  Serial.println("Connected to");
  Serial.println(ssid);
  display.clearDisplay();
  display.display();
  delay(100);
  display.setCursor(10, 0); // Set cursor to top-left
  display.println(product);
  display.setCursor(35, 15);
  display.println(WiFi.localIP());
  display.display();
  delay(100);
  display.display();

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}


// prepare a web page to be send to a client (web browser)
String prepareHtmlPage()
{
  String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
            "\r\n" +
            "<title>ESP-Grow Rev A</title>"
            "<!DOCTYPE HTML>" +
            "<style>"+
            "body{ background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }"
            "</style>"
            "<h1>ESP-GROW Web Server</h1>"+
            "<h2>"+"By: Kyle Rodrigues | "+String(revision)+"</h2>"
            "<html>" +
            "Soil Moisture Content(%):  " + String(soilval)+"<br>"+
            "Air Temperature(%):  " + String(t)+"<br>"+
            "Humidity(%):  " + String(h)+"<br>"+"<br>"+"<br>"+
            "System Uptime:  "+String(Hour)+":"+String(Minute)+":"+String(Second)+"<br>"+
            "</html>" +
            "\r\n";
  return htmlPage;
}


void loop(){
  checksensor();
  if(digitalRead(btn)==LOW){
    shortwaterpump();
  }
  uptime();

  WiFiClient client = server.available();
  // wait for a client (web browser) to connect
  if (client)
  {
    Serial.println("\n[Client connected]");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
          client.println(prepareHtmlPage());
          break;
        }
      }
    }
    delay(1); // give the web browser time to receive the data

    // close the connection:
    client.stop();
    Serial.println("[Client disonnected]");
  }
}

void checksensor(){
soilval =  map(analogRead(0),320,760,100,0);
  h = dht.readHumidity();
  t = dht.readTemperature();
  display.clearDisplay();
  display.setCursor(10, 0); // Set cursor to top-left
  display.println(product);
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
  
  if(soilval<40){
    waterpump();
  }
}

void waterpump(){
    digitalWrite(pump, HIGH);
    delay(5000);
    digitalWrite(pump, LOW);
  }

  void shortwaterpump(){
    digitalWrite(pump, HIGH);
    delay(1000);
    digitalWrite(pump, LOW);
  }

//this part does the uptime
 void uptime(){
//** Making Note of an expected rollover *****//   
if(millis()>=3000000000){ 
HighMillis=1;

}
//** Making note of actual rollover **//
if(millis()<=100000&&HighMillis==1){
Rollover++;
HighMillis=0;
}

long secsUp = millis()/1000;

Second = secsUp%60;

Minute = (secsUp/60)%60;

Hour = (secsUp/(60*60))%24;

Day = (Rollover*50)+(secsUp/(60*60*24));  //First portion takes care of a rollover [around 50 days]
                       
};

