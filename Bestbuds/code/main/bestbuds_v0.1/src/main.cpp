/*
--------------------------------------
BestBuds Firmware Version 1.0
Date: 12/07/2019
Author: Brad Momberg, Kyle Rodrigues
Firmware Block Diagram: [insert link]
--------------------------------------
Processor: ESP32 
HSPI Pins: 
ADC0_CS = IO4;
ADC1_CS = IO16;
ADC2_CS = IO33;
SCK = IO14
MISO = IO12
MOSI = IO13
https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/spi_master.html
*
*/


//---Prebuild Libraries
  #include <Arduino.h>
  #include <FirebaseESP32.h>
  #include <Dps310.h>
  #include <SPI.h>
  #include <Adafruit_NeoPixel.h>
  #include <Adafruit_SSD1306.h>
  #include <Adafruit_GFX.h>
  #include <rom/rtc.h>

  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>

  #include <NTPClient.h>

  #include <EEPROM.h>

//--Custom Libraries
  #include "push_to_firebase.h"     //NEEDS TO BE COMPLETED
  #include "pressure_sensor.h"      //NEEDS TO BE COMPLETED
  #include "ADS7961.h"
  #include "DHTesp.h"
  #include "pH_Sensor.h"
  #include "bitmaps.h"



//---Variables for new devices/rollouts
  const String  customerID = "Genesis";
  const int     roomNumber = 1;
  const String  HarvestID   = "2019-BD-001";  
  const String  plantStrain ="Blue_Dream";
  int deviceNum;
  const String Moisture_Stake_Version = "1.0";  
  const String Module_Firmware_Version = "v1.01";



//---WiFi Objects EP32 DOES NOT LIKE UNDERSCORES!!!
  //Change these for each new router to be set up
  //Home Router
  //#define WIFI_SSID "Pretty Fly For A Wifi-2.4"
  //#define WIFI_PASSWORD "supercarbon"

//Genisis Router
  #define WIFI_SSID "supercarbon-2G"
  #define WIFI_PASSWORD "runner7373"

//---NTP Client Stuff
  //-These objects are used to the the Unix timestamp into the Firebase push stuff.
  //
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);
  unsigned long epochTime;

//---Neopixel Variables and Objects
  #define PIN 18 //Neopixel pins
  #define NUMPIXELS   48 // number of pixels
  // Create an array with the current status of the neopixel
  int PixelStatus[3][48];
  // When setting up the NeoPixel library, we tell it how many pixels,
  // and which pin to use to send signals. Note that for older NeoPixel
  // strips you might need to change the third parameter -- see the
  // strandtest example for more information on possible values.
  Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ400);
  #define DELAYVAL 1 // Time (in milliseconds) to pause between pixels



#define EEPROM_SIZE 2048

//---Firebase Objects
  //These variables SHOULD NOT change on each customer rollout
  //
  #define FIREBASE_HOST "https://grow-d8f9a.firebaseio.com"
  #define FIREBASE_AUTH "nRlg3MIM5LZ9AGdqlAsMhwrQ6RPi12UsJr0XsbSa"

  FirebaseData firebaseData;
  FirebaseJson jsonModuleData;  
  FirebaseJson jsonEnviroData;
  FirebaseJson jsonSoilData;
  FirebaseJsonArray jsonArray_SoilMoisture;

//---Function Prototypes
  //
  int connect_Wifi();
  void firebase_Data_Push();     
  void init_ADC();    
  void ADCRead(int ADC_NUM);  
  void NeopixelSetup();        
  void NeopixelWrite();
  void DHTSetup();
  void CheckDHT();
  void ScreenSetup();
  void ScreenUpdate();
  void pHRead();
  void print_reset_reason(RESET_REASON);
  void verbose_print_reset_reason(RESET_REASON);
  void grabChipId();
  void OTA_init();
  void timeStamp_init();
  void timeStamp_grab();

  void readSerialCalibration();
  void mediumMoistureCalibration();
  void getOffsetCalibration();

  double readCelsius(void);
  double readFahrenheit(void);

  void ADCCalibrate_Dry(int ADC_NUM);
  void ADCCalibrate_Wet(int ADC_NUM);

  void pressureSensorRead();

  void readSerial();
  void pHsensorCalibration();
  void pHRead();

  void setDeviceNumber();
  //Bitmaps
  void WIFI_bitmap();
  void pHCal_bitmap();
  void loading_bitmap();
  void temp_bitmap();
  void pressure_bitmap();
  void ADC_bitmap();
  void time_bitmap();
  
  int loading_status; //this is used for the loading bitmap switch case

//---Moisture Sensor Variables
  //---How many moistures stakes are there
  //
  const int NUM_STAKES        =  16;
  const int NUM_ADC_CHANNELS  =  3;
  /*This array gets used in the ADC_MUX_Update function
  The intention is to built out a 2 dimensional array to store the values from the ADC
  The the ADC Mux spits back a SPI packet that includes the channel that was just read and the value of the channel
  In the ADC_Mux_Update() function, the array index will be compared with the ADC channel value to ensure they are in sync
  */
  int Medium_Moisture_Measurements[NUM_STAKES][NUM_ADC_CHANNELS] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}, {16,17,18}, {19,20,21}
                                                                    ,{22,23,24}, {25,26,27},{28,29,30},{31,32,33}, {34,35,36},{37,38,39},{40,41,42},{43,44,45}
                                                                    ,{46,47,48} };


  //The % humidity values of the moisture stake
  int Medium_Moisture_Percentage[NUM_STAKES][NUM_ADC_CHANNELS] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}, {16,17,18}, {19,20,21}
                                                                    ,{22,23,24}, {25,26,27},{28,29,30},{31,32,33}, {34,35,36},{37,38,39},{40,41,42},{43,44,45}
                                                                    ,{46,47,48} };

  //Calibration variables for the dry value (Maximum)
  int Dry_Offset_Calibration[NUM_STAKES][NUM_ADC_CHANNELS] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}, {16,17,18}, {19,20,21}
                                                                    ,{22,23,24}, {25,26,27},{28,29,30},{31,32,33}, {34,35,36},{37,38,39},{40,41,42},{43,44,45}
                                                                    ,{46,47,48} };

  //Calibration variables for the wet value (Maximum)
  int Wet_Offset_Calibration[NUM_STAKES][NUM_ADC_CHANNELS] = {{1,2,3}, {4,5,6}, {7,8,9}, {10,11,12}, {13,14,15}, {16,17,18}, {19,20,21}
                                                                    ,{22,23,24}, {25,26,27},{28,29,30},{31,32,33}, {34,35,36},{37,38,39},{40,41,42},{43,44,45}
                                                                    ,{46,47,48} };

  

//---Pressure Sensor Objects
  //
  
  #define CS_PRESSURE_SENSOR 32           
  Dps310 Dps310PressureSensor = Dps310();
  const int Pressure_CS = 32;      // set SPI SS Pin
  float Dps_Barometric_Pressure;
  float Dps_Temperature;

  

//---Thermocouple ChipSelect
  const int Thermocouple_CS = 17;      // set SPI SS Pin
  double medium_Temperature; //store the vaue

//---MAX6675 Thermocouple Varibles
  uint16_t RawTemperature;
  const int TempOffSet = -3;

//Debug Function Variables  
  uint64_t chipID_short, chipid; 
  #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

//---pH Variables
 //See pH_sensor.h

//------DHT variables
  float DHT_airTemperature, DHT_relativeHumidity;
  DHTesp dht;


//---OLED Definitions and Object
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels
  #define OLED_RESET     1 // // Wemo SCL 
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



unsigned long upTime_Timer=0;
                              //Minutes * Seconds * Milliseconds
unsigned long upTime_MaxDuration = 10 * 60 * 1000;

struct sensors {
  bool isPresent = false;
  float value;
};

struct sensors phProbe;
struct sensors thermoCouple;


void setup() {
  Serial.begin(115200);
  delay(200);
  //Init Neopixel
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  NeopixelSetup();// Load Default Values into NeoPixel strip 
  

  //---EEPROM Initialization
  EEPROM.begin(EEPROM_SIZE);

  //initialise the instance of the SPIClass attached to HSPI 
  hspi = new SPIClass(HSPI);   //initialise hspi
  hspi->begin();
  pinMode(ADC0_CS, OUTPUT); //HSPI SS for ADC#
  pinMode(ADC1_CS, OUTPUT); //HSPI SS for ADC#
  pinMode(ADC2_CS, OUTPUT); //HSPI SS for ADC#
  
  //Thermocouple ChipSelect
  pinMode(Thermocouple_CS, OUTPUT);
  digitalWrite(Thermocouple_CS, HIGH);

  setDeviceNumber();
 
  //OLED Screen Setup
  ScreenSetup();
  delay(10);

  pinMode(CS_PRESSURE_SENSOR, OUTPUT);
 // Serial.println("HSPI Setup Complete");

  //Pressure Sensor 
  Dps310PressureSensor.begin(*hspi, CS_PRESSURE_SENSOR);
  pinMode(Pressure_CS, OUTPUT);
  digitalWrite(Pressure_CS, HIGH); 

  delay(100);

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

  //OTA_init() cannot be removed. Without it, we won't be able to publish another update
  OTA_init();

  delay(100);

  //SPACE RESERVED FOR SCREEN SPLASH
  /*The screen splash is only going to be triggered if the device reset cause is not a software reset*/
  delay(100);
  

  init_ADC();
  delay(100);

  DHTSetup();
  delay(100);


  readSerialCalibration(); 

  getOffsetCalibration();

  /*Improve this function by causing it to read multiple samples over a period of time and average the results*/
  if(phProbe.isPresent){
      pHCal_bitmap();
      pHRead(); 
      delay(500);    
  }

  if(thermoCouple.isPresent){
      temp_bitmap();
      medium_Temperature = readCelsius();
      if(isnan(medium_Temperature)){
        medium_Temperature = 0;
        Serial.println("Medium Temperature out of bounds. \t Setting 0");
      }
  }
  /*The check DHT function will be improved by building it into the firebase_Data_Push function
  We will be calling CheckDHT and passing the temperature values directly*/
  pressure_bitmap();
  CheckDHT();  
  delay(500);

  pressureSensorRead();

  /*This function should be improved by adding averaging to the results sampled*/
  ADC_bitmap();
  Serial.println("---ADC Saple run 1---");
  ADCRead(ADC0_CS); 
  ADCRead(ADC1_CS);
 // ADCRead(ADC2_CS);
  delay(100);

  //Added for testing on 11/26/2019
  //Attempting to fix bug #16
  Serial.println("---ADC Sample Run 2---");
  ADCRead(ADC0_CS); 
  ADCRead(ADC1_CS);
  delay(100);

  /*Neopixel write*/
  Serial.println("Neopixel Write");
  NeopixelWrite(); 


  /*This function contains the offset for the timezone stuff*/
   Serial.println("Fetching Time Stamp");
  time_bitmap();
  timeStamp_init();
  timeStamp_grab();

  /* Firebase write*/
  //Pushing to firebase should be the last thing we do.
  //Because it's pretty much the last functional part of the program
  firebase_Data_Push();  

  upTime_Timer = millis();
  
}


void loop() {

  ArduinoOTA.handle();

  /*We should create a function to loop around and update the OLED display with info
  This will give the customer the impression that the device is doing things when it's
  not actually sending off samples to Firebase*/
  ScreenUpdate();

  if(phProbe.isPresent)
      pHRead();

  if(thermoCouple.isPresent){
      medium_Temperature = readCelsius();
      if(isnan(medium_Temperature)){
        medium_Temperature = 0;
        Serial.println("Medium Temperature out of bounds. \t Setting 0");
      }
  }

  //Added for testing on 11/6/2019
  ADCRead(ADC0_CS); 
  ADCRead(ADC1_CS);
  //ADCRead(ADC2_CS);
  delay(100);

  NeopixelWrite();

 // Serial.print("Medium Reading %: "); Serial.println(Medium_Moisture_Measurements[0][0]);

  Serial.print("Uptime (s): ");       Serial.print(millis()/1000);
  Serial.print(" of: "); Serial.println( (upTime_Timer+upTime_MaxDuration) / 1000 );
  delay(5000);  



  if(upTime_Timer + upTime_MaxDuration < millis()){
    Serial.print("---Device Restarting---");
    ESP.restart();
  }

}


//Make the  function more robust by having it run through for up to 30 seconds before exiting
int connect_Wifi(){
  int wifi_stat_bar = 0;
  uint16_t currentMillis = millis();
  uint16_t timerMaxDuration = 15000;

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        /*
        if(wifi_stat_bar<128){
        display.fillRect(0, 0, wifi_stat_bar, 15, WHITE);
        display.display();
        wifi_stat_bar++;
        }*/
        delay(300);
        if(currentMillis + timerMaxDuration < millis()){
          Serial.println("WiFi Connection Failed: Timeout");
          break;
          return 0;
        }
    }
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("Signal Strenght: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" RSSI");
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    return 1;
}

//This is a test function
void firebase_Data_Push(){

  //#######---WiFi and Firebase setup
    //
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);

   // String path_1 = "firmware_testing/11_11/"; //This path is used for testing purposes...Change the date whenever working with this function
  //  String path_1 = "Firmware_Testing/11_28/"; //This path is used for testing purposes...Change the date whenever working with this function
  // String path_1 = "Firmware_testing/12_04/";
    String path_2 = customerID + "/" + plantStrain + "/" + roomNumber + "/" + deviceNum + "/";
    String path_3 = HarvestID + "/" + epochTime;    


    jsonArray_SoilMoisture.addInt(Medium_Moisture_Percentage[0][0]).addInt(Medium_Moisture_Percentage[1][0]).addInt(Medium_Moisture_Percentage[2][0]).addInt(Medium_Moisture_Percentage[3][0])
      .addInt(Medium_Moisture_Percentage[4][0]).addInt(Medium_Moisture_Percentage[5][0]).addInt(Medium_Moisture_Percentage[6][0]).addInt(Medium_Moisture_Percentage[7][0])
      .addInt(Medium_Moisture_Percentage[8][0]).addInt(Medium_Moisture_Percentage[9][0]).addInt(Medium_Moisture_Percentage[10][0]).addInt(Medium_Moisture_Percentage[11][0])
      .addInt(Medium_Moisture_Percentage[12][0]).addInt(Medium_Moisture_Percentage[13][0]).addInt(Medium_Moisture_Percentage[14][0]).addInt(Medium_Moisture_Percentage[15][0])
      .addInt(Medium_Moisture_Percentage[0][1]).addInt(Medium_Moisture_Percentage[1][1]).addInt(Medium_Moisture_Percentage[2][1]).addInt(Medium_Moisture_Percentage[3][1])
      .addInt(Medium_Moisture_Percentage[4][1]).addInt(Medium_Moisture_Percentage[5][1]).addInt(Medium_Moisture_Percentage[6][1]).addInt(Medium_Moisture_Percentage[7][1])
      .addInt(Medium_Moisture_Percentage[8][1]).addInt(Medium_Moisture_Percentage[9][1]).addInt(Medium_Moisture_Percentage[10][1]).addInt(Medium_Moisture_Percentage[11][1])
      .addInt(Medium_Moisture_Percentage[12][1]).addInt(Medium_Moisture_Percentage[13][1]).addInt(Medium_Moisture_Percentage[14][1]).addInt(Medium_Moisture_Percentage[15][1])
    ;

    //---OLD CODE
    // jsonEnviroData.addDouble("Air_temp", DHT_airTemperature).addDouble("Relative_Humidity", DHT_relativeHumidity).addDouble("Baro_Pressure", Dps_Barometric_Pressure);
    //Check to see if the values grabbed from the DHT are real numbers, if they're not: Send NaN to Firebase;
    if(isnan(DHT_airTemperature)){
       jsonEnviroData.addDouble("Air_temp", 0);
       Serial.println("Dps Air Temperature: NAN. \tSetting 0");
     }
      else{
        jsonEnviroData.addDouble("Air_temp",DHT_airTemperature);
      }

    if(isnan(DHT_relativeHumidity)){
      jsonEnviroData.addDouble("Relative_Humidity", 0);
       Serial.println("DHT Air Humidity: NAN. \tSetting 0");
      }
      else{
          jsonEnviroData.addDouble("Relative_Humidity",DHT_relativeHumidity);
      }

    if(isnan(Dps_Barometric_Pressure)){
      jsonEnviroData.addDouble("Baro_Pressure", 0);
       Serial.println("Dps Baro Pressure: NAN. \tSetting 0");
      }
      else{
        jsonEnviroData.addDouble("Baro_Pressure", Dps_Barometric_Pressure);
      }


    jsonSoilData.addArray("Medium_Moisture", &jsonArray_SoilMoisture);  

    if(thermoCouple.isPresent){
      temp_bitmap();
      double medium_Temperature = readCelsius();
      if(isnan(medium_Temperature)){
        medium_Temperature = 0;
        Serial.println("Medium Temperature out of bounds. \t Setting 0");
      }
        jsonSoilData.addDouble("Medium_Temp",medium_Temperature);
      }
      else{
        Serial.println("-Firebase Push- Medium thermocouple not present on device");
      }

    if(phProbe.isPresent){
        if(isnan(pH_sensor_value))
          pH_sensor_value = 0;
      jsonSoilData.addDouble("Medium_pH", pH_sensor_value);
      }
      else{
        Serial.println("-Firebase Push- pH probe not present on device");
      }

    jsonModuleData.clear().addJson("Enviro_Data",&jsonEnviroData).addJson("Medium_Data",&jsonSoilData);

    //if (Firebase.setJSON(firebaseData, path_1 + path_2 + path_3, jsonModuleData))
    if (Firebase.setJSON(firebaseData, path_2 + path_3, jsonModuleData))
    {
        Serial.println("PASSED");
        Serial.println("PATH: " + firebaseData.dataPath());
        Serial.println("TYPE: " + firebaseData.dataType());
    }
    else
    {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }
    //#######---WiFi and Firebase set up and testing routine complete
}

//----------Neopixel---------------
void NeopixelSetup(){
  for (int i = 0; i < NUMPIXELS; i++)
  {
    PixelStatus[0][i] = 0;   //set default red value to 0
    PixelStatus[1][i] = 50;  //set default green value to 10
    PixelStatus[2][i] = 10;   //set default blue value to 0
  }

  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();
  delay(DELAYVAL);
  
}

void NeopixelWrite() {
 
  int MOISTURE_THRESHOLD = 1000;
  //This function checks for missing stakes
  for(int i = 0; i < 16; i++){
    if(Medium_Moisture_Measurements[i][0] < MOISTURE_THRESHOLD ){
      Serial.print("Pixel "); Serial.print(i); Serial.println(" - No Stake");
      pixels.setPixelColor(i, pixels.Color(0,50,0));
    } else
    {
      Serial.print("Pixel "); Serial.print(i); Serial.println(" - Stake Present");
      pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
      portDISABLE_INTERRUPTS();      // Neopixel timing workaround
      pixels.show();   // Send the updated pixel colors to the hardware.
      portENABLE_INTERRUPTS();
      delay(DELAYVAL); // Pause before next pass through loop
   }

     for(int i = 16; i < 32; i++){
    if(Medium_Moisture_Measurements[i-16][1] < MOISTURE_THRESHOLD ){
      Serial.print("Pixel "); Serial.print(i); Serial.println(" - No Stake");
      pixels.setPixelColor(i, pixels.Color(0,50,0));
    } else
    {
      Serial.print("Pixel "); Serial.print(i); Serial.println(" - Stake Present");
      pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
      portDISABLE_INTERRUPTS();      // Neopixel timing workaround
      pixels.show();   // Send the updated pixel colors to the hardware.
      portENABLE_INTERRUPTS();
      delay(DELAYVAL); // Pause before next pass through loop
   }
     /*for(int i = 0; i < 16; i++){

      if(Medium_Moisture_Percentage[i][1] < MOISTURE_THRESHOLD){
      //Serial.print("Pixel "); Serial.print(i); Serial.println(" active");
      pixels.setPixelColor(i, pixels.Color(0,50,0));
      portDISABLE_INTERRUPTS();     // Neopixel timing workaround
      pixels.show();   // Send the updated pixel colors to the hardware.
      portENABLE_INTERRUPTS();

    delay(DELAYVAL); // Pause before next pass through loop
    //delayMicroseconds(DELAYVAL);
    }
  }

  /*for(int i = 16; i < 32; i++){

      if(Medium_Moisture_Percentage[i-16][1] < MOISTURE_THRESHOLD){
      //Serial.print("Pixel "); Serial.print(i); Serial.println(" active");
      pixels.setPixelColor(i, pixels.Color(0,50,0));
      portDISABLE_INTERRUPTS();     // Neopixel timing workaround
      pixels.show();   // Send the updated pixel colors to the hardware.
      portENABLE_INTERRUPTS();

    delay(DELAYVAL); // Pause before next pass through loop
    //delayMicroseconds(DELAYVAL);
    }
  }*/
  Serial.println("Neopixels updated");
}

void init_ADC(){
  //ADC0 Setup
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  //Send dummy byte
  digitalWrite(ADC0_CS, LOW);
 // Serial.println("ADC0-Dummy toggle");
  hspi->transfer16(0);
  digitalWrite(ADC0_CS, HIGH);
  //Send Prog Reg
  digitalWrite(ADC0_CS, LOW);
  Serial.println("ADC0-HSPI Sending ProgReg:");
 // Serial.println(ProgReg,BIN);
  hspi->transfer16(ProgReg); 
  digitalWrite(ADC0_CS, HIGH);
  //Set Channels to monitor amd exclude
  digitalWrite(ADC0_CS, LOW);
  Serial.println("ADC0-HSPI Sending Mode Ctrl Reg");
 // Serial.println(ModeCtrlReg,BIN);
  hspi->transfer16(ModeCtrlReg);
  digitalWrite(ADC0_CS, HIGH);
  hspi->endTransaction();

  //ADC1 Setup
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  //Send dummy byte
  digitalWrite(ADC1_CS, LOW);
  Serial.println("ADC1-Dummy toggle");
  hspi->transfer16(0);
  digitalWrite(ADC1_CS, HIGH);
  //Send Prog Reg
  digitalWrite(ADC1_CS, LOW);
  Serial.println("ADC1-HSPI Sending ProgReg:");
  //Serial.println(ProgReg,BIN);
  hspi->transfer16(ProgReg); 
  digitalWrite(ADC1_CS, HIGH);
  //Set Channels to monitor amd exclude
  digitalWrite(ADC1_CS, LOW);
  Serial.println("ADC1-HSPI Sending Mode Ctrl Reg");
  //Serial.println(ModeCtrlReg,BIN);
  hspi->transfer16(ModeCtrlReg);
  digitalWrite(ADC1_CS, HIGH);
  hspi->endTransaction();

  //ADC2 Setup
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  //Send dummy byte
  digitalWrite(ADC2_CS, LOW);
  Serial.println("ADC2-Dummy toggle");
  hspi->transfer16(0);
  digitalWrite(ADC2_CS, HIGH);
  //Send Prog Reg
  digitalWrite(ADC2_CS, LOW);
  Serial.println("ADC2-HSPI Sending ProgReg:");
  //Serial.println(ProgReg,BIN);
  hspi->transfer16(ProgReg); 
  digitalWrite(ADC2_CS, HIGH);
  //Set Channels to monitor amd exclude
  digitalWrite(ADC2_CS, LOW);
  Serial.println("ADC2-HSPI Sending Mode Ctrl Reg");
  //Serial.println(ModeCtrlReg,BIN);
  hspi->transfer16(ModeCtrlReg);
  digitalWrite(ADC2_CS, HIGH);
  hspi->endTransaction();
}

//The ADC read function holds the integer mapping function onto the array that we push to firebase
void ADCRead(int ADC_NUM) {
  switch (ADC_NUM) //Sets the array column numbers
    {
    case 4:
      ADC_Colu = 0;
      break;

    case 16:
      ADC_Colu = 1;
      break;  

      case 33:
      ADC_Colu = 2;
      break;

    default:
      break;
    }
  Serial.println("- - - ADC READ FUNCTION - - -");
  for (int i = 0; i < 16; i++){
    hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    
    //Read Data from regsiters
    digitalWrite(ADC_NUM, LOW);
    spi_bytes_raw = hspi->transfer16(0);
    digitalWrite(ADC_NUM, HIGH);
    hspi->endTransaction();
    
    CHVal = (spi_bytes_raw >> 12);
    CHVal_dec = CHVal;
    
    delay(10);

    //Serial.print("ADC Colum  ");        Serial.print(ADC_Colu);
    //Serial.print("\t Channel Value:  ");   Serial.println(CHVal,DEC);

    ChVolt_bin = (spi_bytes_raw & 0b0000111111111111);
   // ChVolt_bin = (ChVolt_bin >> 4);       //USe this line for 8 bit ADC

   Medium_Moisture_Measurements[CHVal_dec][ADC_Colu]    = ChVolt_bin;
    
  // Serial.print("Dry_Offset: "); Serial.print(Dry_Offset_Calibration[CHVal][ADC_Colu]); 
  // Serial.print("\t Wet_Offset: ");  Serial.println(Wet_Offset_Calibration[CHVal][ADC_Colu]);

   Medium_Moisture_Percentage[CHVal_dec][ADC_Colu]      = map(ChVolt_bin,Dry_Offset_Calibration[CHVal][ADC_Colu], Wet_Offset_Calibration[CHVal][ADC_Colu], 0, 100);   //Mapping function requires testing
  // Medium_Moisture_Percentage[CHVal_dec][ADC_Colu]      = map(ChVolt_bin,3700, 2300, 0, 100);   //Mapping function requires testing
   
   
   Serial.print("CHVal: "); Serial.print(CHVal); Serial.print("\t ADC_Colu: "); Serial.print(ADC_Colu);
   Serial.print("\tVal%: ");   Serial.println(Medium_Moisture_Percentage[CHVal][ADC_Colu]);
   Serial.print("  Val#: ");   Serial.println(Medium_Moisture_Measurements[CHVal][ADC_Colu]);

   if(Medium_Moisture_Percentage[CHVal][ADC_Colu] > 100){
    // Serial.println("Medium_Moisture_Percentage Out of Range - - - Assigning '100'");
     Medium_Moisture_Percentage[CHVal][ADC_Colu] = 100;
   }
   else if(Medium_Moisture_Percentage[CHVal][ADC_Colu] < 0){
     Medium_Moisture_Percentage[CHVal][ADC_Colu] = 0;
   }
   
  }
}

void DHTSetup(){
  //The dht.setup() function could be improved to return a 1 if succesful instead of being a void function
  //DHT Startup
  dht.setup(27, DHTesp::DHT22); // Connect DHT22 sensor to GPIO 2
}

void CheckDHT(){
  Serial.println("DHT air temperature and humidity");
  delay(dht.getMinimumSamplingPeriod());
  DHT_relativeHumidity = dht.getHumidity();
  DHT_airTemperature = dht.getTemperature();
  Serial.print("DHT Temp: ");
  Serial.print(DHT_airTemperature);
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

void ScreenUpdate(){
      /*This function should be improved to make the text more readable from a distance*/
  display.clearDisplay();
  display.setCursor(0, 0); // Set cursor to top-left
  display.println(HarvestID);
  display.setCursor(90, 0);
  display.println(Module_Firmware_Version); 
  display.setCursor(0, 20);
  display.println(WiFi.localIP()); 
  display.setCursor(90, 20);
  display.print(WiFi.RSSI());
  display.setCursor(10, 30);
  display.println("ADC0: ");  
  display.setCursor(40, 30);
  display.println(Medium_Moisture_Measurements[0][0]);
  display.setCursor(10, 40);
  display.println("pH: ");  
  display.setCursor(30, 40);
  if ((0.0<pH_sensor_value)&&(pH_sensor_value<14.0)) {
    display.println(pH_sensor_value);
  }else
  {
    display.println("No Probe");
  }
  display.setCursor(0, 50);
  display.println("Thermo: ");
  display.setCursor(45, 50);
  display.println(medium_Temperature);
  display.setCursor(75, 50);
  display.println("oC"); 
  display.display();       // Refresh the display
  delay(100);  
  display.display();       // Refresh the display
}

void pHsensorCalibration(){
  pHCal_bitmap();
  float pH_cal_low_volt ;
  float pH_cal_high_volt;
  int delaySeconds = 120;

  const unsigned int pH_addr_low  = 1000;
  const unsigned int pH_addr_high  = 1010;

  Serial.println("---pH Sensor Calibration Routine---");
  Serial.println("Follow the prompts to calibrate the sensors");

  int incomingByte = 0;
  bool breakFlag = false;
  //---Low pH Calibration Routine
  Serial.println("--- Low pH Calibration Routine ---");
  Serial.println("Please insert the sensor in a 4.00 pH buffer solution");
  Serial.println("Press Y to continue");
  do{
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte)
      {
      case 121: //121 == Y
        pHCal_bitmap();
        Serial.print("Stablizing");
        for(int i = 0; i < delaySeconds; i = i+10){
            if(i<128){
              display.fillRect(0, 0, i, 15, WHITE);
              display.display();
             }
          Serial.print(".");
          delay(10000);
        }
        for (int i = 0; i < 10; i++){
          pH_cal_low_buf[i]=analogRead(pH_Analog_Pin);
          }
        for (int i = 0; i < 10; i++){
          pH_cal_low_raw += pH_cal_low_buf[i];
        }
        pH_cal_low_raw = pH_cal_low_raw/10;
        Serial.println("-Low Calibration Complete");
        Serial.print("Averaged Low ADC Reading: ");
        Serial.println(pH_cal_low_raw);
        Serial.print(" ");

        pH_cal_low_volt = (pH_cal_low_raw*5.000)/4096; 
        Serial.print("Voltage Reading: ");
        Serial.println(pH_cal_low_volt);
        Serial.print(" ");

        EEPROM.put(pH_addr_low, pH_cal_low_volt);
        EEPROM.commit();

        Serial.println("Low pH Value Calibrated");
        breakFlag = true;
        break;
      
      default:
        Serial.println("Enter Y when ready to calibrate");
        break;
      }
    }
  }while(breakFlag == false);

  breakFlag = false;
  //---High pH Calibration Routine
  Serial.println("--- High pH Calibration Routine ---");
  Serial.println("Please insert the sensor in a 10.00 pH buffer solution");
  Serial.println("Press Y to continue");
  pHCal_bitmap();
  do{
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte)
      {
        case 121: //121 == Y
          Serial.print("Stablizing");
          for(int i = 0; i < delaySeconds; i = i+10){
              if(i<128){
                display.fillRect(0, 0, i, 15, WHITE);
                display.display();
              }
            Serial.print(".");
            delay(10000);
            }
          for (int i = 0; i < 10; i++){
            pH_cal_high_buf[i]=analogRead(pH_Analog_Pin);
            }
          for (int i = 0; i < 10; i++){
            pH_cal_high_raw += pH_cal_high_buf[i];
          }
          pH_cal_high_raw = pH_cal_high_raw/10;
          Serial.print("Averaged High ADC Reading: ");
          Serial.println(pH_cal_high_raw);
          Serial.print(" ");

          pH_cal_high_volt = (pH_cal_high_raw*5.000)/4096;
          Serial.print("Voltage Reading: ");
          Serial.println(pH_cal_high_volt);
          Serial.print(" ");

          EEPROM.put(pH_addr_high, pH_cal_high_volt);
          EEPROM.commit();

          Serial.println("High pH Value Calibrated");
          breakFlag = true;
          break;
      
          default:
        Serial.println("Enter Y when ready to calibrate for dry");
        break;
      }
    }
    
    }while(breakFlag == false);
  //linear interpolation
  pHSlope = (pH_cal_high_volt-pH_cal_low_volt)/(10.000-4.000);
 
}

void pHRead(){

  float pH_cal_low_volt;
  float pH_cal_high_volt;

  const unsigned int pH_addr_low  = 1000;
  const unsigned int pH_addr_high  = 1010;

  EEPROM.get(pH_addr_low, pH_cal_low_volt);
  EEPROM.get(pH_addr_high,pH_cal_high_volt);

  Serial.print("pH_cal_low_volt: "); Serial.print(pH_cal_low_volt);
  Serial.print("\tpH_cal_high_volt: "); Serial.println(pH_cal_high_volt);

  pHSlope = (pH_cal_high_volt-pH_cal_low_volt)/(10.000-4.000);
  Serial.print("pH Slope Reading: ");
  Serial.println(pHSlope);
  Serial.print(" ");
  pH_read_raw = 0;

  for (int i = 0; i < 20; i++){
    pH_read_buf[i]=analogRead(pH_Analog_Pin);
    //Serial.println(i);
     //Serial.println(pH_read_buf[i]);
    pH_read_raw += pH_read_buf[i];
    //Serial.println(pH_read_raw);
    //Serial.println(" ");
    delay(10);
  }
 
  pH_read_raw = pH_read_raw/20;
  Serial.print("Averaged ADC Reading: ");
  Serial.println(pH_read_raw);
  Serial.print(" ");
      
  pH_read_volt = (pH_read_raw*5.000)/4095;
  Serial.print("Voltage Reading: ");
  Serial.println(pH_read_volt);
  Serial.print(" ");    

  pH_sensor_value = ((pH_read_volt-pH_cal_low_volt)+(pHSlope*4.00)-pH_cal_offset)/pHSlope;
  Serial.print("pH Sensor Value = ");
  Serial.println(pH_sensor_value);
  Serial.println(" ");
  phProbe.value = pH_sensor_value;
}

void print_reset_reason(RESET_REASON reason){
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET");break;          /**<1,  Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}

void verbose_print_reset_reason(RESET_REASON reason){
  switch ( reason)
  {
    case 1  : Serial.println ("Vbat power on reset");break;
    case 3  : Serial.println ("Software reset digital core");break;
    case 4  : Serial.println ("Legacy watch dog reset digital core");break;
    case 5  : Serial.println ("Deep Sleep reset digital core");break;
    case 6  : Serial.println ("Reset by SLC module, reset digital core");break;
    case 7  : Serial.println ("Timer Group0 Watch dog reset digital core");break;
    case 8  : Serial.println ("Timer Group1 Watch dog reset digital core");break;
    case 9  : Serial.println ("RTC Watch dog Reset digital core");break;
    case 10 : Serial.println ("Instrusion tested to reset CPU");break;
    case 11 : Serial.println ("Time Group reset CPU");break;
    case 12 : Serial.println ("Software reset CPU");break;
    case 13 : Serial.println ("RTC Watch dog Reset CPU");break;
    case 14 : Serial.println ("for APP CPU, reseted by PRO CPU");break;
    case 15 : Serial.println ("Reset when the vdd voltage is not stable");break;
    case 16 : Serial.println ("RTC Watch dog reset digital core and rtc module");break;
    default : Serial.println ("NO_MEAN");
  }
}

void setDeviceNumber(){

    //Unique ESP32 Chip ID
  chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
	Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  chipID_short = chipid>>32;

  Serial.print("chipID_Short: "); Serial.println((uint32_t)chipID_short);


  switch((uint32_t)chipID_short){
    case 15455:
      deviceNum = 1;
      phProbe.isPresent = true;
      thermoCouple.isPresent = true;
      break;
    case 32855:
      deviceNum = 2;
      phProbe.isPresent = true;
      thermoCouple.isPresent = true;
      break;
    case 21591:
      deviceNum = 3;
      phProbe.isPresent = true;
      thermoCouple.isPresent = true;
      break;
    case 17495:
      deviceNum = 4;
      phProbe.isPresent = true;
      thermoCouple.isPresent = true;
      break;
    case 36951:
      deviceNum = 5;
      break;
    case 13399:
      deviceNum = 6;
      break;
    case 12383:
      deviceNum = 7;
      break;
    case 17503:
      deviceNum = 8;
      break;
    case 13407:
      deviceNum = 9;
      break;
    case 84:
      deviceNum = 10;
      break;
    case 38999:
      deviceNum = 11;
      break;
    case 13396:
      deviceNum = 12;
      break;
    default:
      deviceNum = 100;
      break;
  }
  Serial.print("deviceNum: "); Serial.println(deviceNum);

  

}

void OTA_init(){

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

    ArduinoOTA.begin();

}

void timeStamp_init(){
  timeClient.begin();

  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  // GMT -8 = -28800
  timeClient.setTimeOffset(-28800);
}

void timeStamp_grab(){

    while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  epochTime = timeClient.getEpochTime();
  Serial.println(epochTime);

}


void readSerialCalibration(){

    unsigned const int timerStart = millis();
    unsigned const int timerMaxDuration = 10000;

    int incomingByte = 0;
    bool breakFlag = false;

    Serial.println("---Calibration Routines---");
    Serial.print("1 - Moisture Calibration \t");
    if(phProbe.isPresent)
      Serial.print("2 - pH Calibration \t");
    Serial.println("N - Continue to main program");

  do{

    if((timerStart + timerMaxDuration) < millis() ){
        Serial.println("Timer Elapsed - Continuing to Main Program");
        break;
    }

    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      //Serial.print("incomingByte: "); Serial.println(incomingByte);
    
      switch (incomingByte)
      {
      case 49:  //49 == 1
          Serial.println("Entered Calibration Routine");
          mediumMoistureCalibration();
        break;
      case 50:  //50 == 2
          Serial.print("pH Calibration Routine");
          if(phProbe.isPresent)
            pHsensorCalibration();
          else{
            Serial.println("pH probe not present");
          }
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


void mediumMoistureCalibration(){

  Serial.println("---Sensor Calibration Routine---");
  Serial.println("Follow the prompts to calibrate the sensors");

    int incomingByte = 0;
    bool breakFlag = false;
  //---Dry Calibration Routine
  Serial.println("--- Dry Calibration Routine ---");
  Serial.println("Ensure the sensors are in the open and free of moisture");
  Serial.print("Press Y to continue");    Serial.print("Press any key to skip Dry");
  do{
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte)
      {
      case 121: //121 == Y
        ADCCalibrate_Dry(4);
        ADCCalibrate_Dry(16);
        Serial.println("Dry Value Calibrated");
        breakFlag = true;
        break;
      

      default:
        Serial.println("Skipping Dry");
        breakFlag = true;
        break;
      }
    }
  }while(breakFlag == false);

  delay(200);

  breakFlag = false;
  //---Wet Calibration Routine
  Serial.println("--- Wet Calibration Routine ---");
  Serial.println("Ensure the sensors are placed up to the white line in water");
  Serial.println("Press Y to continue");
  do{
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();

      switch (incomingByte)
      {
      case 121: //121 == Y
        ADCCalibrate_Wet(4);
        ADCCalibrate_Wet(16);
        Serial.println("Wet Value Calibrated");
        breakFlag = true;
        break;
      
      default:
        Serial.println("Enter Y when ready to calibrate for dry");
        break;
      }
    }
    }while(breakFlag == false);
}

//- - - 
void ADCCalibrate_Dry(int ADC_NUM) {

  /*
  Dry   ADC0: 0   - 150
        ADC1: 160  - 310
  Wet   ADC0: 320 - 470
        ADC1: 480 - 630 
  */

  int eeprom_Address_Counter = 0;
  switch (ADC_NUM) //Sets the array column numbers
  {
  case 4:
    ADC_Colu = 0;
    eeprom_Address_Counter = 0;
    Serial.println("ADC 0 - Calibrating");
    break;

  case 16:
    ADC_Colu = 1;
    eeprom_Address_Counter = 16;
    Serial.println("ADC 1 - Calibrating");
    break;  

    case 33:
    ADC_Colu = 2;
    break;

  default:
    break;
  }

  //Changed the loop to run 17 times instead of 16
    for (int i = 0; i < 17; i++){
      hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
      
      //Read Data from regsiters
      digitalWrite(ADC_NUM, LOW);
      spi_bytes_raw = hspi->transfer16(0);
      digitalWrite(ADC_NUM, HIGH);
      hspi->endTransaction();
      
      CHVal = (spi_bytes_raw >> 12);
      CHVal_dec = CHVal;
    
      ChVolt_bin = (spi_bytes_raw & 0b0000111111111111);

      Dry_Offset_Calibration[CHVal_dec][ADC_Colu]    = ChVolt_bin;
      
      int address =  ((16*ADC_Colu) + CHVal) * 10;   

      EEPROM.put(address, Dry_Offset_Calibration[CHVal][ADC_Colu]);
      EEPROM.commit();
  

      Serial.print("Channel: "); Serial.print(CHVal);
      Serial.print("\tCalibrated Val: ");    Serial.print(Dry_Offset_Calibration[CHVal][ADC_Colu]);  
      Serial.print("\tStored in address: "); Serial.println(address);  

    }
}

void ADCCalibrate_Wet(int ADC_NUM) {

  /*
  Dry   ADC0: 0   - 150
        ADC1: 160  - 310
  Wet   ADC0: 320 - 470
        ADC1: 480 - 630 
  */
  int eeprom_Address_Counter;
  switch (ADC_NUM) //Sets the array column numbers
    {
    case 4:
      ADC_Colu = 0;
      eeprom_Address_Counter = 32;
      Serial.println("ADC 0 - Calibrating");
      break;

    case 16:
      ADC_Colu = 1;
      eeprom_Address_Counter = 48;
      Serial.println("ADC 1 - Calibrating");
      break;  

      case 33:
      ADC_Colu = 2;
      break;

    default:
      break;
    }

  for (int i = 0; i < 16; i++){
    hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    
    //Read Data from regsiters
    digitalWrite(ADC_NUM, LOW);
    spi_bytes_raw = hspi->transfer16(0);
    digitalWrite(ADC_NUM, HIGH);
    hspi->endTransaction();
    
    CHVal = (spi_bytes_raw >> 12);
    CHVal_dec = CHVal;
  
    ChVolt_bin = (spi_bytes_raw & 0b0000111111111111);

   Wet_Offset_Calibration[CHVal_dec][ADC_Colu]    = ChVolt_bin;

   int address =  ((16*ADC_Colu) + CHVal) * 10;   

    EEPROM.put((address + 320 ), Wet_Offset_Calibration[CHVal][ADC_Colu]);
    EEPROM.commit();

    Serial.print("Channel: "); Serial.print(CHVal); 
    Serial.print("\tCalibrated Val: ");    Serial.print(Wet_Offset_Calibration[CHVal][ADC_Colu]);    
    Serial.print("\tStored in address: "); Serial.println(address + 320 );
  }
}

void   getOffsetCalibration(){

  Serial.println("- - - Moisture Offset Calibraion - EEPROM GET - - -");

  for(int i = 0; i < 16; i++){
  //  Dry_Offset_Calibration[i][0] = EEPROM.readInt(i*10);
    EEPROM.get(i*10,Dry_Offset_Calibration[i][0]);
  //     Serial.print("i= "); Serial.print(i);
  //     Serial.print("\toutputEEPROM Address: "); Serial.print(i*10);  
  //     Serial.print("\tEEPROM_Read: "); Serial.print(EEPROM.readInt(i*10));
  //     Serial.print("\t Dry_Offset: "); Serial.println(Dry_Offset_Calibration[i][0]);    
   }
  for(int i = 16; i < 32; i++){
    //Dry_Offset_Calibration[i-16][1] = EEPROM.readInt(i*10);
     EEPROM.get(i*10,Dry_Offset_Calibration[i-16][1]);
      // Serial.print("i= "); Serial.print(i);
      // Serial.print("\toutputEEPROM Address: "); Serial.print(i*10);  
      // Serial.print("\tEEPROM_Read: "); Serial.print(EEPROM.readInt(i*10));
      // Serial.print("\t Dry_Offset: "); Serial.println(Dry_Offset_Calibration[i-16][1]);
  }
  for(int i = 32; i < 48; i++){
    //Wet_Offset_Calibration[i-32][0] = EEPROM.readInt(i*10);
    EEPROM.get(i*10, Wet_Offset_Calibration[i-32][0]);
      // Serial.print("i= "); Serial.print(i);
      // Serial.print("\toutputEEPROM Address: "); Serial.print(i*10);  
      // Serial.print("\tEEPROM_Read: "); Serial.print(EEPROM.readInt(i*10));
      // Serial.print("\t Wet_Offset: "); Serial.println(Wet_Offset_Calibration[i-32][0]);
  }
  for(int i = 48; i < 64; i++){

    EEPROM.get(i*10,Wet_Offset_Calibration[i-48][1]);
      // Serial.print("i= "); Serial.print(i);
      // Serial.print("\toutputEEPROM Address: "); Serial.print(i*10);  
      // Serial.print("\tEEPROM_Read: "); Serial.print(EEPROM.readInt(i*10));
      // Serial.print("\t Wet_Offset: "); Serial.println(Wet_Offset_Calibration[i-48][1]);
  }

  delay(2000);
  
}

double readCelsius(void) {
  
  hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(Thermocouple_CS, LOW);
  // CSB Fall to Output Enable
  delayMicroseconds(1000);
  RawTemperature = hspi->transfer16(0);
  digitalWrite(Thermocouple_CS, HIGH);
  hspi->endTransaction();
  // CSB Rise to Output Disable
  delayMicroseconds(1000);
  //Uncomment this to read raw
  //Serial.println(RawTemperature,BIN);
  if (RawTemperature & 0x4) {
    // uh oh, no thermocouple attached!
    Serial.println("No Thermocouple");
    return NAN;
    //return -100;
  }

  RawTemperature >>= 3;

  //return RawTemperature*0.25;
  return ((RawTemperature*0.25)+TempOffSet);
}

double readFahrenheit(void) {
  return readCelsius() * 9.0/5.0 + 32;
}

void pressureSensorRead()
{
  float temperature;
  float pressure;
  uint8_t oversampling = 7;
  int16_t ret;
  Serial.println();

  //lets the Dps310 perform a Single temperature measurement with the last (or standard) configuration
  //The result will be written to the paramerter temperature
  //ret = Dps310PressureSensor.measureTempOnce(temperature);
  //the commented line below does exactly the same as the one above, but you can also config the precision
  //oversampling can be a value from 0 to 7
  //the Dps 310 will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
  //measurements with higher precision take more time, consult datasheet for more information
  //ret = Dps310PressureSensor.measureTempOnce(temperature, oversampling);
  ret = Dps310PressureSensor.measureTempOnce(Dps_Temperature, oversampling);
  if (ret != 0)
  {
    //Something went wrong.
    //Look at the library code for more information about return codes
    Serial.print("DPS Failure - Temperature --- ret: ");
    Serial.println(ret);
    Dps_Temperature = 0;
  }
  else
  {
    Serial.print("DPS Temperature: ");
    Serial.print(Dps_Temperature);
    Serial.println(" degrees of Celsius");
  }


  //ret = Dps310PressureSensor.measurePressureOnce(pressure);
  //ret = Dps310PressureSensor.measurePressureOnce(pressure, oversampling);
  ret = Dps310PressureSensor.measurePressureOnce(Dps_Barometric_Pressure, oversampling);
  Dps_Barometric_Pressure = Dps_Barometric_Pressure / 1000; //Convert to KPa instead of Pa
  if (ret != 0)
  {
    //Something went wrong.
    //Look at the library code for more information about return codes
    Serial.print("DPS Failure - Pressure --- ret: ");
    Serial.println(ret);
    Dps_Barometric_Pressure = 0;
  }
  else
  {
    Serial.print("Pressure: ");
    Serial.print(Dps_Barometric_Pressure);
    Serial.println(" Pascal");
  }

  //Wait some time
  delay(1000);
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
    
  void pressure_bitmap(void){
    display.clearDisplay();
    display.drawBitmap(
    (display.width()  - pressure_width ) / 2,
    (display.height() - pressure_height) / 2,
    pressure_bmp, pressure_width, pressure_height, 1);
    display.display();
  }

  void ADC_bitmap(void){
      display.clearDisplay();
      display.drawBitmap(
      (display.width()  - ADC_width ) / 2,
      (display.height() - ADC_height) / 2,
      ADC_bmp, ADC_width, ADC_height, 1);
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