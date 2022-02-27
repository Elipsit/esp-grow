/*
Calibration Code moving here
*/

#include <Arduino.h>
#include <EEPROM.h>
#include "main.h"
#include "calibrate.h"

SoilCal SensStake = {786,914,100,110,0,0};
pump_h PUMP = {0,120};

void SoilSens_Init(){
  EEPROM.get(SensStake.soil_addr_dry,SensStake.DrySensCal);
  EEPROM.get(SensStake.soil_addr_wet,SensStake.WetSensCal);
  EEPROM.get(PUMP.pump_threshold_addr,PUMP.pump_threhold);
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
    SensStake.cal_dry_raw = 0;
  
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
            SensStake.cal_dry_raw += cal_dry_buf[i];
          }

        SensStake.DrySensCal = SensStake.cal_dry_raw/10;
        Serial.print("Averaged Dry Sensor ADC Reading: ");
        Serial.println(SensStake.DrySensCal);
        Serial.print(" ");
        
        EEPROM.put(SensStake.soil_addr_dry,SensStake.DrySensCal);
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
    SensStake.cal_wet_raw = 0;
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
          SensStake.cal_wet_raw += cal_wet_buf[i];
          }

        SensStake.WetSensCal = SensStake.cal_wet_raw/10;
        Serial.print("Averaged Wet Sensor ADC Reading: ");
        Serial.println(SensStake.WetSensCal);
        Serial.print(" ");

        EEPROM.put(SensStake.soil_addr_wet,SensStake.WetSensCal);
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
        PUMP.pump_threhold = 20;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 50:  //50 == 2
        Serial.println("Waterpump threshold =  25%");
        PUMP.pump_threhold = 25;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 51:  //51 == 3
        Serial.println("Waterpump threshold =  30%");
        PUMP.pump_threhold = 30;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 52:  //52 == 4
        Serial.println("Waterpump threshold =  35%");
        PUMP.pump_threhold = 35;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 53:  //53 == 5
        Serial.println("Waterpump threshold =  40%");
        PUMP.pump_threhold = 40;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 54:  //54 == 6
        Serial.println("Waterpump threshold =  45%");
        PUMP.pump_threhold = 45;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 55:  //55 == 7
        Serial.println("Waterpump threshold =  50%");
        PUMP.pump_threhold = 50;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 56:  //56 == 8
        Serial.println("Waterpump threshold =  55%");
        PUMP.pump_threhold = 55;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
        EEPROM.commit();
        breakFlag = true;
        break;

        case 57:  //56 == 9
        Serial.println("Waterpump threshold =  60%");
        PUMP.pump_threhold = 60;
        EEPROM.put(PUMP.pump_threshold_addr,PUMP.pump_threhold);
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