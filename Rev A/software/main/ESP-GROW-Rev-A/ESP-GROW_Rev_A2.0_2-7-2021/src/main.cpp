/* ___________________________________
 |   ESP-Grow Rev A                   |
 |   By Kyle Rodrigues                |
 |   Date: 3/20/2020                  |
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
*3/21/2020
*
****Status***
*works but doesn't load well on mobile. 
 */
//****Include Statements*****
#include <Arduino.h>
#include <EEPROM.h>
#include "main.h"
#include "bitmaps.h"


//OLED Screen
#include <Wire.h>  // Include Wire if you're using I2C
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//DHT22
#include "DHTesp.h"
//ESP8266
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

//****Definitions*****
//*****OLED 0.96" **********
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET     -1 // // Wemo SCL 
//#define OLED_RESET     1 // // Wemo SCL 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//EEPROM
#define EEPROM_SIZE 2048


int WetSensCal, DrySensCal;
const unsigned int soil_addr_wet  = 100;
const unsigned int soil_addr_dry  = 110;
int cal_wet_buf[10];
int cal_dry_buf[10];
long unsigned int cal_dry_raw, cal_wet_raw;
int pump_threhold;
const unsigned int pump_threshold_addr = 120;


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

//******Serial Flags*******
    int incomingByte = 0;
    bool breakFlag = false;


//****Function Definitions***********
int connect_Wifi();
void ScreenSetup();
void waterpump();
void shortwaterpump();
void checksensor();
void uptime();
void debug();
void wifiStatus();
String prepareHtmlPage();
void RunWifiClient();
void readSerial();
void SensorCal();
void DrySensorCal();
void WetSensorCal();
void PumpThreshold();
void ScreenUpdate();
void setDeviceNumber();

 //Bitmaps
void WIFI_bitmap();
 void pHCal_bitmap();
 void loading_bitmap();
 void temp_bitmap();
 void pressure_bitmap();
 void ADC_bitmap();
 void time_bitmap();

WiFiServer server(80);

//This function controlls the auto water function
void waterpump(){
  if(AutoPumpMaxCount>6){
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
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  delay(100);
  
  Serial.println("Serial Started");

  //Start EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("EEPROM Done");
  delay(1000);

  //Revision
  Serial.println(revision);
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

  //Get CHIPID
  setDeviceNumber();

 //GPIO
  pinMode(pump, OUTPUT);
  pinMode(btn, INPUT);
  pinMode(scrbtn, INPUT);
  Serial.println("GPIO Started");

  
  //******OLED Start*****
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
   Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }
  
  Serial.println("Screen Started");
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
  Serial.println("DHT Started");

  //Serial.printf("Connecting to %s ", ssid);

  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
     {
       if (digitalRead(scrbtn) == LOW)
        {
         debug();
        }else
         { 
          int WiFi_Connect_Attempts = 0;
          int WiFi_Connect_Max_Attempts = 6;
          WIFI_bitmap();
          do{
            connect_Wifi();
            WiFi_Connect_Attempts++;
            if(WiFi_Connect_Attempts > WiFi_Connect_Max_Attempts){
            Serial.println("--- WIFI CONNECTION FAILED---RESTARTING DEVICE---");
            ESP.restart();
            }
              }while(WiFi.status() !=  WL_CONNECTED);
           loading_bitmap();
        }
     }  
      /*
      display.setCursor(40, 20);
      display.println("No WIFI");
      display.display();
      delay(100);
      display.display();
      */
     
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

  EEPROM.get(soil_addr_dry,DrySensCal);
  EEPROM.get(soil_addr_wet,WetSensCal);
  EEPROM.get(pump_threshold_addr,pump_threhold);
  Serial.println("Current Calibration Data: ");
  Serial.print("Dry: ");
  Serial.print(DrySensCal);
  Serial.print("\tWet: ");
  Serial.print(WetSensCal);
  Serial.print("\tPump Threshold: ");
  Serial.print(pump_threhold);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  readSerial();
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
    ScreenUpdate();
    Serial.printf("\nChipID: %u \t Product: %s", CHIPID,product);
    Serial.printf("\nPump Threshold: %d %% \t Soil Moisture: %d %% \tPump Daily Cycle: %d", pump_threhold,soilval,AutoPumpMaxCount);
    Serial.printf("\nDHT: %4.2f oC \t%4.2f %%RH", t,h);
    Serial.printf("\nUptime: %d:%d:%d:%d", Day,Hour,Minute,Second);
    Serial.println(" ");
    delay(1000);  
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

void ScreenSetup(){
  //******OLED Start*****
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
   // for(;;); // Don't proceed, loop forever
  }
    display.display();
   // delay(2000); // Pause for 2 seconds
    delay(2000);    // Clear the buffer
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(20, 0); // Set cursor to top-left
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // Print can be used to print a string to the screen:
    display.println("Super Carbon");
    display.display();       // Refresh the display
    delay(100);  
}

void debug (){
  while (1)
  {
  soilval_raw = analogRead(0);
  soilval =  (soilval_raw,WetSensCal,DrySensCal,100,0);
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
  
  if((soilval<pump_threhold)&(soilval>5)){
    waterpump();
  }
   delay(1000);
  }
}

void checksensor(){
  soilval_raw = analogRead(0);
  soilval =  map(soilval_raw,WetSensCal,DrySensCal,100,0);
  delay(dht.getMinimumSamplingPeriod());
   h = dht.getHumidity();
   t = dht.getTemperature();
  
  if((soilval<pump_threhold)&(soilval>5)){
    waterpump();
  }
}

void ScreenUpdate(){
  display.clearDisplay();
  display.setCursor(0, 0); // Set cursor to top-left
  display.println(product);
  //Display the IPAddress
  display.setCursor(10, 20);
  display.println(WiFi.localIP());
  //Displays the RSSI Value
  display.setCursor(90, 0);
  display.print(WiFi.RSSI());

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
  //Update Screen with DHT22 Temperature and Humidity
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

void readSerial(){

    unsigned const int timerStart = millis();
    unsigned const int timerMaxDuration = 10000;

    incomingByte = 0;
    breakFlag = false;
    loading_bitmap(); //add a laoding splash on the screen
    Serial.println("---Soil Sensor Calibration Routine---");
    Serial.println("1 - Enter Sensor Calibration Routine \t 2 - Pump Threshold \t N - Continue to main program");
    Serial.println("");

  do{

    if((timerStart + timerMaxDuration) < millis() ){
       Serial.println("Timer Elapsed - Continuing to Main Program");
      break;
    }

    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
  
      switch (incomingByte){
        case 49:  //49 == 1
        Serial.println("Entered Calibration Routine");
        SensorCal();
        break;

        case 50:  //50 == 2
        Serial.println("Entered Pump Threshold Routine");
        PumpThreshold();
        break;
      
        case 110: //110 = n
        Serial.println("Continued to main program");
        breakFlag = true;
        break;
        
        default:
        Serial.println("Invalid input");
        Serial.println("Y - Enter Routine \t N - Continue to main program");
        break;
      }

    }

    }while(breakFlag == false);
}

void SensorCal(){ 
  breakFlag = false;
  incomingByte = 0;
  Serial.println("Soil Sensor Calibration");
  Serial.println("1 - Enter Dry Routine \t 2 - Enter Wet Routine\t N - Continue to main program");
  Serial.println("");
  do{
  if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
  
      switch (incomingByte){
        case 49:  //49 == 1
        Serial.println("Enter Dry Calibration Routine");
        DrySensorCal();
        break;

        case 50:  //50 == 2
        Serial.println("Enter Wet Calibration Routine");
        WetSensorCal();
        break;
      
        case 110: //110 = N
        Serial.println("Continued to main program");
        breakFlag = true;
        break;
        
        default:
        Serial.println("Invalid input");
        Serial.println("1 - Enter Dry Routine \t 2 - Enter Wet Routine \n N - To Continue To Main Program");
        break;
      }

    }

    }while(breakFlag == false);
}


 void DrySensorCal(){
    incomingByte = 0;
    breakFlag = false;
    cal_dry_raw = 0;
  
    //---Dry Soil Calibration Routine
    Serial.println("--- Dry Sensor Calibration Routine ---");
    Serial.println("Please insert the sensor into a dry medium");
    Serial.println("Press Y to continue");
    do{
      if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte){
        case 121: //121 == Y
          for (int i = 0; i < 10; i++){
            cal_dry_buf[i]=analogRead(A0);
            cal_dry_raw += cal_dry_buf[i];
          }

        DrySensCal = cal_dry_raw/10;
        Serial.print("Averaged Dry Sensor ADC Reading: ");
        Serial.println(DrySensCal);
        Serial.print(" ");
        
        EEPROM.put(soil_addr_dry,DrySensCal);
        EEPROM.commit();
        Serial.println("Dry Sensor Value Calibrated");
        breakFlag = true;
      break;

        case 110: //110 = n
          Serial.println("Continued to main program");
          breakFlag = true;
          break;
    
    default:
      Serial.println("Enter Y when ready to calibrate for Wet Sensor");
      break;
    }
  }
  }while(breakFlag == false);

  }

void WetSensorCal(){
    incomingByte = 0;
    breakFlag = false;
    cal_wet_raw = 0;
  //---Wet Sensor Calibration Routine
  Serial.println("--- Wet Sensor Calibration Routine ---");
  Serial.println("Please insert the sensor in a wet medium");
  Serial.println("Press Y to continue");
  do{
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte)
      {
      case 121: //121 == Y
        for (int i = 0; i < 10; i++){
          cal_wet_buf[i]=analogRead(A0);
          cal_wet_raw += cal_wet_buf[i];
          }

        WetSensCal = cal_wet_raw/10;
        Serial.print("Averaged Wet Sensor ADC Reading: ");
        Serial.println(WetSensCal);
        Serial.print(" ");

        EEPROM.put(soil_addr_wet,WetSensCal);
        EEPROM.commit();

      Serial.println("Wet Sensor Value Calibrated");
      breakFlag = true;
      break;

      case 110: //110 = n
        Serial.println("Continued to main program");
        breakFlag = true;
        break;
    
    default:
      Serial.println("Enter Y when ready to calibrate for Wet Sensor");
      break;
    }
  }
  }while(breakFlag == false);

}

void PumpThreshold(){
 Serial.println("---Pump Threshold Routine---");
  Serial.println("Select number with the corresponding soil mositure threshold");
  Serial.println("1 = 20% | 2 = 25% | 3 = 30% | 4 = 35% | 5 = 40% | 6 = 45% | 7 = 50% | 8 = 55% ");
  incomingByte = 0;
  breakFlag = false;
  delay(500);
  do{
  if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
  
      switch (incomingByte){
          case 49:  //49 == 1
        Serial.println("Waterpump threshold =  20%");
        pump_threhold = 20;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 50:  //50 == 2
        Serial.println("Waterpump threshold =  25%");
        pump_threhold = 25;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 51:  //51 == 3
        Serial.println("Waterpump threshold =  30%");
        pump_threhold = 30;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 52:  //52 == 4
        Serial.println("Waterpump threshold =  35%");
        pump_threhold = 35;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 53:  //53 == 5
        Serial.println("Waterpump threshold =  40%");
        pump_threhold = 40;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 54:  //54 == 6
        Serial.println("Waterpump threshold =  45%");
        pump_threhold = 45;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 55:  //55 == 7
        Serial.println("Waterpump threshold =  50%");
        pump_threhold = 50;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 56:  //56 == 8
        Serial.println("Waterpump threshold =  55%");
        pump_threhold = 55;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 57:  //56 == 9
        Serial.println("Waterpump threshold =  60%");
        pump_threhold = 60;
        EEPROM.put(pump_threshold_addr,pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;
        
        default:
        Serial.println("Invalid input");
        Serial.println("Continue to main program");
        breakFlag = true;
        break;
      }

    }
  }while(breakFlag == false);

}

int connect_Wifi(){
  int wifi_stat_bar = 0;
  uint16_t currentMillis = millis();
  uint16_t timerMaxDuration = 15000;

    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        if(wifi_stat_bar<128){
        display.fillRect(0, 0, wifi_stat_bar, 15, WHITE);
        display.display();
        wifi_stat_bar++;
        delay(300);
        if(currentMillis + timerMaxDuration < millis()){
          Serial.println("WiFi Connection Failed: Timeout");
          break;
          return 0;
          }
        }
    }
}

void WIFI_bitmap(void) {
  display.clearDisplay();
  display.drawBitmap(
  (display.width()  - WIFI_width ) / 2,
  (display.height() - WIFI_height) / 2,
  WIFI_bmp, WIFI_width, WIFI_height, 1);
  display.display();
}

void pHCal_bitmap(void) {
  display.clearDisplay();
  display.drawBitmap(
  (display.width()  - pHCal_width ) / 2,
  (display.height() - pHCal_height) / 2,
  pHCal_bmp, pHCal_width, pHCal_height, 1);
  display.display();
}

  void loading_bitmap(void){
    display.clearDisplay();
    display.drawBitmap(
    (display.width()  - loading_width ) / 2,
    (display.height() - loading_height) / 2,
    loading_bmp, loading_width, loading_height, 1);
    display.display();
  }

  void temp_bitmap(void){
    display.clearDisplay();
    display.drawBitmap(
    (display.width()  - temp_width ) / 2,
    (display.height() - temp_height) / 2,
    temp_bmp, temp_width, temp_height, 1);
    display.display();
  }
    
  void time_bitmap(void){
      display.clearDisplay();
      display.drawBitmap(
      (display.width()  - time_width ) / 2,
     (display.height() - time_height) / 2,
     time_bmp, time_width, time_height, 1);
     display.display();
  }


void setDeviceNumber(){

  Serial.print("CHIPID: "); Serial.println(CHIPID);
  switch(CHIPID){
    case 2885726208:
      product = "TrainWreck";
      break;
    case 654356480:
      product = "Romulan";
      break;

    default:
       break;
  }
}
