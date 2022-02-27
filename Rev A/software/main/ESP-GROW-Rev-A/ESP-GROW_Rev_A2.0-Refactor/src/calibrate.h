#ifndef CALIBRATE_H
#define CALIBRATE_H


int cal_wet_buf[10];
int cal_dry_buf[10];

//------Serial Flags------
int incomingByte = 0;
bool breakFlag = false;

struct SoilCal {
    int WetSensCal;
    int DrySensCal;
    int soil_addr_wet;
    int soil_addr_dry;
    long unsigned int cal_dry_raw;
    long unsigned int cal_wet_raw;
};

struct pump_h {
    int pump_threhold;
    const unsigned int pump_threshold_addr;
};

extern SoilCal SensStake;
extern pump_h PUMP;

void SoilSens_Init();
void SensorCal();
void DrySensorCal();
void WetSensorCal();
void PumpThreshold();


#endif