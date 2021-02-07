/* ___________________________________
 |   ESP-Grow Rev A                   |
 |   By Kyle Rodrigues                |
 |   Date: 8/04/2019                   |
  ___________________________________
***Library Versions:***
See platform.ini
*
***Notes***
-Use the GPIO# pins for mapping.
-Make sure to add these:
http://arduino.esp8266.com/stable/package_esp8266com_index.json
*
***Pin Assignment***   
DHT22 - GPIO2 (D4)
OLED Screen -GPIO4, GPIO5, Addr 0x3C, character uses 5x5 pixels, (X,Y)
soil capacitive sensor - A0
Water Pump - GPIO16
Pump Button - GPIO12
Screen Button - GPIO0
*
*9/8/2019
*soil sensor setpoint changed to int on main.h
*new usb type soil sensors used in this test
*water short chaged to 5sec long changed to 10sec
*debug mode removes soil sensor error
****Status***
*works but doesn't load well on mobile. 
 */
//****Include Statements*****
#include <Arduino.h>
#include "main.h"
//OLED Screen
#include <Wire.h>  // Include Wire if you're using I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//DHT22
//#include "DHT.h"
#include "DHTesp.h"
//ESP8266
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

//****Definitions*****
//*****OLED 0.96" **********
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     1 // // Wemo SCL 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int  soilval, pumpcyc;

float t,h;
const int pump = 16; // water pump
const int btn = 12; //push button
const int scrbtn = 0; //push button
int soilval_raw;
int AutoPumpMaxCount=0;

DHTesp dht;

//***Uptime deff***
long Day=0;
int Hour =0;
int Minute=0;
int Second=0;
int HighMillis=0;
int Rollover=0;


//****Function Definitions***********
void waterpump();
void shortwaterpump();
void checksensor();
void uptime();
void debug();
void wifiStatus();
String prepareHtmlPage();
void RunWifiClient();

WiFiServer server(80);

//This function controlls the auto water function
void waterpump(){
  if(AutoPumpMaxCount>3){
    //do nothing
    Serial.println("Pump has reached max daily runtime");
  }else{
    digitalWrite(pump, HIGH);
    delay(15000);
    digitalWrite(pump, LOW);
    pumpcyc++;
    AutoPumpMaxCount++;
  }
}

//This function is the manual water pump
void shortwaterpump(){
    digitalWrite(pump, HIGH);
    delay(5000);
    digitalWrite(pump, LOW);
    pumpcyc++;
}
  
void setup(){
  Serial.begin(115200);
  Serial.println();
  
  //Revision
  Serial.println(revision);
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

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
  delay(100); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(30, 0); // Set cursor to top-left
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  // Print can be used to print a string to the screen:
  display.println(product);
  display.display();       // Refresh the display
  delay(100);          // Delay a second and repeat

  //DHT Startup
  //dht.setup(2, DHTesp::DHT11); // Connect DHT11 sensor to GPIO 2
  dht.setup(2, DHTesp::DHT22); // Connect DHT22 sensor to GPIO 2

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
   {
     if (digitalRead(scrbtn) == LOW)
     {
       debug();
     }else
     {
      delay(400);
      Serial.print(".");
      display.setCursor(40, 20);
      display.println("No WIFI");
      display.display();
      delay(100);
      display.display();
     }   

  }
  Serial.println("Connected to");
  Serial.println(ssid);
  display.clearDisplay();
  display.display();
  delay(100);
  display.setCursor(30, 0); // Set cursor to top-left
  display.println(product);
  display.setCursor(10, 20);
  display.println(WiFi.localIP());
  display.display();
  delay(100);
  display.display();

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

void loop(){
  if (Day > 1)
   ESP.restart();
  
  checksensor();
  if(digitalRead(btn)==LOW){
    shortwaterpump();
  }else if (digitalRead(scrbtn)==LOW){
    wifiStatus();
  }else{
    uptime();
    RunWifiClient();  
    }
  }


void RunWifiClient(){
  WiFiClient client = server.available();
  // wait for a client (web browser) to connect
  if (client){
    Serial.println("\n[Client connected]");
    while (client.connected()) {
      // read line by line what the client (web browser) is requesting
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n') {
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

void wifiStatus(){
  display.clearDisplay();
  display.display();
  delay(100);
  display.display();
  display.setCursor(10, 0); // Set cursor to top-left
  display.println("WIFI Settings");
  display.setCursor(0, 20);
  display.println("SSID:");
  display.setCursor(0, 30);
  display.println(ssid);
  display.setCursor(00, 40);
  display.println("Pasphrase:");
  display.setCursor(0, 50);
  display.println(password);
  display.display();
  delay(100);
  display.display();
  delay(5000);
  
}

void debug (){
  while (1)
  {
  soilval_raw = analogRead(0);
  soilval =  (soilval_raw,sensmap_wet,sensmap_dry,100,0);
  delay(dht.getMinimumSamplingPeriod());
   h = dht.getHumidity();
   t = dht.getTemperature();

  display.clearDisplay();
  display.setCursor(20, 0); // Set cursor to top-left
  display.println("Debug Mode");
  
   //Soil Capacitance Sensor
  display.setCursor(0, 30);
  display.println("Soil:");  
  display.setCursor(35, 30);
  display.println(soilval_raw);  
 
   
  //DHT22 Temperature and Humidity
  display.setCursor(0, 40);
  display.println("DHT22:");
  display.setCursor(35, 40);
  display.println(t,1);
  display.setCursor(70, 40);
  display.println("oC");
  display.setCursor(35, 50);
  display.println(h,1);
  display.setCursor(70, 50);
  display.println("%");  
  display.display();
  delay(100);
  display.display();
  
  if((soilval<soilsetpoint)&(soilval>5)){
    waterpump();
  }
   delay(1000);
  }
}

void checksensor(){
  soilval_raw = analogRead(0);
  soilval =  map(soilval_raw,sensmap_wet,sensmap_dry,100,0);
  delay(dht.getMinimumSamplingPeriod());
   h = dht.getHumidity();
   t = dht.getTemperature();

  display.clearDisplay();
  display.setCursor(30, 0); // Set cursor to top-left
  display.println(product);
  display.setCursor(10, 20);
  display.println(WiFi.localIP());
  
   //Soil Capacitance Sensor
  display.setCursor(0, 30);
   if(soilval < 5){ 
     display.println("No Soil Measured"); //really dry soil or not inserted correctly 
   }
   else if(soilval > 120){
     display.println("Soil Sens Error");    //no soil sensor detected
   }
   else{
  display.println("Soil:");  
  display.setCursor(35, 30);
  display.println(soilval);  
  display.setCursor(50, 30);
  display.println("%");
  display.setCursor(80, 30);
  display.println(soilval_raw); 
   }
  //DHT22 Temperature and Humidity
  display.setCursor(0, 40);
  display.println("DHT22:");
  display.setCursor(35, 40);
  display.println(t,1);
  display.setCursor(70, 40);
  display.println("oC");
  display.setCursor(35, 50);
  display.println(h,1);
  display.setCursor(70, 50);
  display.println("%");  
  display.display();
  delay(100);
  display.display();
  
  if((soilval<soilsetpoint)&(soilval>5)){
    waterpump();
  }
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
                       
}

// prepare a web page to be send to a client (web browser)
String prepareHtmlPage(){
  if (soilval < 5)
  {
       String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            //"Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
            "Refresh: 25\r\n" +  // refresh the page automatically every 25 sec
            "\r\n" +
	  "<!DOCTYPE html>" +
	  "<html>" +
	  "<head>" +
	  "<title>" +
	  "ESP-GROW</title>" +
	  "<style>" +
	  "body {background-color:#1f2833;background-repeat:no-repeat;background-position:top left;background-attachment:fixed;}" +
	  "h1{font-family:Comfortaa, serif, Sans-Serif; Color: #66fcf1; background-color:#1f2833;}" +
    "h2{font-family:Comfortaa, serif, font-size:18px;font-style:bold;font-weight:normal;color:#66fcf1;background-color:#1f2833;}" +
	  "p {font-family:Comfortaa, serif;font-size:16px;font-style:normal;font-weight:normal;color:#66fcf1;background-color:#1f2833;}" +
	  "</style>" +
	  "</head>" +
	  "<body>" +
	  "<h1>ESP-Grow Webserver</h1>" +
	  "<h2>By: Kyle R</h2>" +
	  "<h2>"+"Current Plant: "+ String(product) + "</h2>" +"<br>"+
	  "<p>"+"Soil Moisture(%): "+ "Soil Sensor Error - Dry soil or Sensor Not Inserted Correctly " + "</p>" +
	  "<p>Air Temperature(oC)" + String(t) + "</p>"+
	  "<p>Humidity(%): " + String(h) +"</p>"+
	  "<p>Pump Cycles: " +	String(pumpcyc)+"</p>" + "<br>" +"<br>"+
	  "<p>Uptime: " +	 String(Hour) + ":" + String(Minute) + ":" + String(Second) +"</p>"+"<br>"+"<br>"+
    "<p>"+"Version: "+ String(revision) + "</p>" +
	  "</body>" +
	  "</html>" +
	  "\r\n";
  return htmlPage;
  }else if (soilval > 110)
  {
        String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            //"Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
            "Refresh: 25\r\n" +  // refresh the page automatically every 25 sec
            "\r\n" +
	  "<!DOCTYPE html>" +
	  "<html>" +
	  "<head>" +
	  "<title>" +
	  "ESP-GROW</title>" +
	  "<style>" +
	  "body {background-color:#1f2833;background-repeat:no-repeat;background-position:top left;background-attachment:fixed;}" +
	  "h1{font-family:Comfortaa, serif, Sans-Serif; Color: #66fcf1; background-color:#1f2833;}" +
    "h2{font-family:Comfortaa, serif, font-size:18px;font-style:bold;font-weight:normal;color:#66fcf1;background-color:#1f2833;}" +
	  "p {font-family:Comfortaa, serif;font-size:16px;font-style:normal;font-weight:normal;color:#66fcf1;background-color:#1f2833;}" +
	  "</style>" +
	  "</head>" +
	  "<body>" +
	  "<h1>ESP-Grow Webserver</h1>" +
	  "<h2>By: Kyle R</h2>" +
	  "<h2>"+"Current Plant: "+ String(product) + "</h2>" +"<br>"+
	  "<p>"+"Soil Moisture(%): "+ "Sensor Error - No sensor detected " + "</p>" +
	  "<p>Air Temperature(oC)" + String(t) + "</p>"+
	  "<p>Humidity(%): " + String(h) +"</p>"+
	  "<p>Pump Cycles: " +	String(pumpcyc)+"</p>" + "<br>" +"<br>"+
	  "<p>Uptime: " +	 String(Hour) + ":" + String(Minute) + ":" + String(Second) +"</p>"+"<br>"+"<br>"+
    "<p>"+"Version: "+ String(revision) + "</p>" +
	  "</body>" +
	  "</html>" +
	  "\r\n";
  return htmlPage;
  }else
  {
    String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            //"Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
            "Refresh: 25\r\n" +  // refresh the page automatically every 25 sec
            "\r\n" +
	  "<!DOCTYPE html>" +
	  "<html>" +
	  "<head>" +
	  "<title>" +
	  "ESP-GROW</title>" +
	  "<style>" +
	  "body {background-color:#1f2833;background-repeat:no-repeat;background-position:top left;background-attachment:fixed;}" +
	  "h1{font-family:Comfortaa, serif, Sans-Serif; Color: #66fcf1; background-color:#1f2833;}" +
    "h2{font-family:Comfortaa, serif, font-size:18px;font-style:bold;font-weight:normal;color:#66fcf1;background-color:#1f2833;}" +
	  "p {font-family:Comfortaa, serif;font-size:16px;font-style:normal;font-weight:normal;color:#66fcf1;background-color:#1f2833;}" +
	  "</style>" +
	  "</head>" +
	  "<body>" +
	  "<h1>ESP-Grow Webserver</h1>" +
	  "<h2>By: Kyle R</h2>" +
	  "<h2>"+"Current Plant: "+ String(product) + "</h2>" +"<br>"+
	  "<p>"+"Soil Moisture(%): "+ String(soilval) + "</p>" +
	  "<p>Air Temperature(oC)" + String(t) + "</p>"+
	  "<p>Humidity(%): " + String(h) +"</p>"+
	  "<p>Pump Cycles: " +	String(pumpcyc)+"</p>" + "<br>" +"<br>"+
	  "<p>Uptime: " +	 String(Hour) + ":" + String(Minute) + ":" + String(Second) +"</p>"+"<br>"+"<br>"+
    "<p>"+"Version: "+ String(revision) + "</p>" +
	  "</body>" +
	  "</html>" +
	  "\r\n";
  return htmlPage;
  }

}