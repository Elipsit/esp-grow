/*
-----------------------------
Project Name: ADS7961 Demo
Date: 9/29/2019
Author: Kyle Rodrigues
-----------------------------
Processor: ESP32 
HSPI Pins: 
ADC0_CS = IO4;
ADC1_CS = IO16;
ADC2_CS = IO33;
SCK = IO14
MISO = IO12
MOSI = IO13
https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/spi_master.html
*Working Code!
*/

//Definitions
#include <Arduino.h>
#include <SPI.h>

//Local 
#include <ADS7961.h>




void setup() {
  Serial.begin(115200);

 //initialise the instance of the SPIClass attached to HSPI 
  hspi = new SPIClass(HSPI);   //initialise hspi
 // hspi->begin(14, 12, 13, ADC_NUM); //SCLK, MISO, MOSI, SS  
  hspi->begin();
  pinMode(ADC_NUM, OUTPUT); //HSPI SS for ADC#
  pinMode(ADC0_CS, OUTPUT); //HSPI SS for ADC#
  pinMode(ADC1_CS, OUTPUT); //HSPI SS for ADC#
  pinMode(ADC2_CS, OUTPUT); //HSPI SS for ADC#
  Serial.println("HSPI Setup Complete");
  delay(100);
}

void ADCprogram(){
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

  for (int i = 0; i < 15; i++){
    hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    //Read Data from regsiters
    digitalWrite(ADC_NUM, LOW);
    spi_bytes_raw = hspi->transfer16(0);
    digitalWrite(ADC_NUM, HIGH);
    hspi->endTransaction();
    
    CHVal = (spi_bytes_raw >> 12);
    CHVal_dec = CHVal;
    Serial.print("ADC Colum  ");
    Serial.println(ADC_Colu);
    Serial.print("Channel Value:  ");
    Serial.println(CHVal,DEC);
    ChVolt_bin = (spi_bytes_raw & 0b0000111111111111);
    ChVolt_bin = (ChVolt_bin >> 4);
    SoilArray[ADC_Colu][CHVal_dec] = ChVolt_bin;
    Serial.print("ADC Pin Value:  ");
    Serial.println(ChVolt_bin);
    /*if ((CHVal+1) != i)
      {
        Serial.print("CHVal_dec:   ");
        Serial.println(CHVal_dec);
        Serial.print("i:   ");
        Serial.println(i);
      //ADC Reset
      i = 0;  // reset counter back to 0 to override bad data
      hspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
      
      //Send dummy byte
      digitalWrite(ADC_NUM, LOW);
      Serial.println("Dummy toggle");
      hspi->transfer16(0);
      digitalWrite(ADC_NUM, HIGH);
      //Send Prog Reg
      digitalWrite(ADC_NUM, LOW);
      Serial.println("HSPI Sending ProgReg:");
      Serial.println(ProgReg,BIN);
      hspi->transfer16(ProgReg); 
      digitalWrite(ADC_NUM, HIGH);
      //Set Channels to monitor amd exclude
      digitalWrite(ADC_NUM, LOW);
      Serial.println("HSPI Sending Mode Ctrl Reg");
      Serial.println(ModeCtrlReg,BIN);
      hspi->transfer16(ModeCtrlReg);
      digitalWrite(ADC_NUM, HIGH);
      hspi->endTransaction();
      }
      else
      {
      SoilArray[ADC_Colu][i] = ChVolt_bin;
      //v_out = ChVolt_bin*(3.3/255);
      //Serial.print("CH Voltage:  ");
      //Serial.println(v_out);
      //Serial.println("....");
      //delay(1000);
      }*/
      
   }
}

void loop() {
  
  ADCprogram(); //set up ADC in the mode we want

  while(1)
  {
     ADCRead(ADC0_CS);
     ADCRead(ADC1_CS);
     ADCRead(ADC2_CS);
    
    Serial.println("  ");
    Serial.println("ADC0:  ");
     for (int i = 0; i < 15; i++)
     {
      Serial.print(SoilArray[0][i]);
      Serial.print(",");
     }
     Serial.println("  ");
     Serial.println("ADC1:   ");
     for (int i = 0; i < 15; i++)
     {
      Serial.print(SoilArray[1][i]);
      Serial.print(",");
     }
     Serial.println("  ");
      Serial.println("ADC2:   ");
     for (int i = 0; i < 15; i++)
     {
      Serial.print(SoilArray[2][i]);
      Serial.print(",");
     }
     
     Serial.println("");
     Serial.println("");
     Serial.println("**********************");
     Serial.println("");
     Serial.println("");
     delay(10000);
  }
      
}