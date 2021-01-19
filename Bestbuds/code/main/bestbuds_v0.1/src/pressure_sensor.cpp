/*#######---Notes---#######
10.14.2019 - We need to swap to using the HSPI library. If that doesn't work, built it outselves.
*/


#include <Dps310.h>

//#define pin_CS 22  

int16_t pin_CS = 22;

Dps310 Pressure_Sensor = Dps310();


//#######---Initialize the Dps310 Barometric Pressure sensor
//
void init_Pressure_Sensor(){
    pinMode(pin_CS, OUTPUT);
    Pressure_Sensor.begin(SPI, pin_CS);

    Serial.println("Pressure Sensor Initialized");
}

//#######---A function that returns the temperature value of the Dps310 Pressure Sensor
//
float pressure_sensor_temperature(){
    Serial.print("---Temperature---");

    float temperature;
    int ret, oversample = 7;

    ret = Pressure_Sensor.measureTempOnce(temperature, oversample);

   //Serial.println(Pressure_Sensor.getProductId());
   int productu_ID = Pressure_Sensor.getProductId();
   Serial.print("Product id: "); Serial.println(productu_ID);

    Serial.print("\t Ret Val: "); Serial.print(ret);
    Serial.print("\t Temp: ");      Serial.println(temperature);
    return temperature;
}


//#######---A function that returns the barometrics pressure value of the Dps310 Pressure sensor
//
float pressure_sensor_barometric(){
    Serial.print("---Pressure---\t");

    float barometric;
    int ret, oversample = 7;

    ret = Pressure_Sensor.measurePressureOnce(barometric, oversample);

    Serial.print("\t Ret Val: "); Serial.print(ret);
    Serial.print("\t Pressure: ");  Serial.println(barometric);
    
    
    return barometric;
}