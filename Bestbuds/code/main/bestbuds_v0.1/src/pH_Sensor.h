/*
*Sensors must be calibrated to work corerectly 
*Code uses equation of the line between two points
*https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/cn0428.html#rd-functionbenefits
*https://www.analog.com/media/en/technical-documentation/tech-articles/Isolated-pH-Monitor-with-Temperature-Compensation.pdf
*https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/cn0326.html#rd-overview
*/

#ifndef pH_Sensor_h

  int pH_cal_low_buf[10];
  int pH_cal_high_buf[10];
  int pH_read_buf[10];
  long unsigned int pH_cal_low_raw, pH_cal_high_raw, pH_read_raw;
  float pH_sensor_value;
  float pHSlope, pH_read_volt;
  float pH_cal_offset = 0.000;
  int pH_Analog_Pin = 35;

#endif