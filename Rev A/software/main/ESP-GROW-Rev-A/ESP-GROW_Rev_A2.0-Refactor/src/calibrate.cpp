/*
Calibration Code moving here
*/

#include <Arduino.h>
#include <EEPROM.h>
#include "main.h"

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