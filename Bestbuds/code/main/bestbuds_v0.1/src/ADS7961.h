/*-----------------------------
ADS7961 Notes:
- 0.5*Vcc voltage divider connected to CHO for test
* 
- Each frame begins with the falling edge of CS.
- With the falling edge of CS, the input signal from the selected channel is sampled, and the conversion process is initiated.
- The device outputs data while the conversion is in progress.
- The 16-bit data word contains a 4-bit channel address, followed by a 12-bit conversion result in MSB first format.
- The device selects a new multiplexer channel on the second SCLK falling edge.
- The acquisition phase starts on the fourteenth SCLK rising edge.
- On the next CS falling edge the acquisition phase will end, and the device starts a new frame.
- Device keeps programmed setting until power reset

Notes: devive boots and cycles through channels in auto 2
*does not provide channel info but voltages correct
*Start > CS = LOW with no transfer CS = HIGH,
* CS = LOW  transfer ProgReg CS = HIGH,
* CS = LOW  transfer ModeCtrlReg CS = HIGH,
*CS = LOW  Read  ADC Chip and store in Ch(n) CS = HIGH
*CS = LOW  Read  ADC Chip and store in Ch(n+1) CS = HIGH 
*repeat for CH0-15
*/

#ifndef ADS7961_H

//Variables
int ADC_NUM, ADC_Colu, CHVal_dec;
const int ADC0_CS = 4;      // set SPI SS Pin
const int ADC1_CS = 16;      // set SPI SS Pin
const int ADC2_CS = 33;      // set SPI SS Pin
int SoilArray [3][16];      // soil sensor ADC array
int cnt;
byte ProgReg = 0b1001; //selects auto mode 2
uint16_t ModeCtrlReg = 0b0011110000000000; //See page 40 datasheet
//uint16_t CHSet = 0b0000000000000111; //monitor channel 0 and ignor rest
uint16_t CHVal;    // channel data after extra bits removed
uint16_t ChVolt_bin; //binary version of adc voltage
int spi_bytes_raw;       // final 12 bit shited value
float v_out;             // decimal voltage
float vref = 3.3;       // voltage on Vref pin

//uninitalised pointers to SPI objects
SPIClass * hspi = NULL;

static const int spiClk = 1000000; // 1 MHz



#endif