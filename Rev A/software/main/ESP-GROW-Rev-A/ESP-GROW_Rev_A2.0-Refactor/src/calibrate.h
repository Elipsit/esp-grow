#ifndef calibrate_h

int WetSensCal, DrySensCal;
const unsigned int soil_addr_wet  = 100;
const unsigned int soil_addr_dry  = 110;
int cal_wet_buf[10];
int cal_dry_buf[10];
long unsigned int cal_dry_raw, cal_wet_raw;

extern void SensorCal();
void DrySensorCal();
void WetSensorCal();


#endif