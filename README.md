# esp-grow
## Project Description
This was a project designed to get fimilar with the ESP8266 and ESP32 chipset.

## esp-grow
### This was an esp8266 based automatic plant monitor system.
The system will monitor the capacitive soil sensor, tenperature and humidity and activate the water pump accordingly.
The system hosts its own webpage to give the user status of the plants progress.

![esp-grow](https://github.com/Elipsit/esp-grow/blob/master/Rev%20A/pics/esp-grow.png)

custom soil sensor
![sens_stake](https://github.com/Elipsit/esp-grow/blob/master/Rev%20B/pics/sens_Stake.png)

## Design Files
### Electrical Design Files
This project contains the Altium Designer schematic and pcb files, wiring diacrams and code for an esp8266 based indoor plant monitoring system.

This system is comprised of three main blocks:
1) Wemos D1 - ESP-12F; I really like working with this form factor since it has an esp8266 with a breadboard header and a CH340G
2) ESP-Grow_OLED_Rev_A: custom 2 layer pcb with a generic SSD1306 128x64 I2C monochrome OLED screen and a tack button seated under the screen.      Basically a screen button.
3) ESP-Grow_OLED_Main_A: The motherboard. A custom 4 Layer pcb with all the connectors, power rails and leds.

Update:
-DHT22 library has an error with ESP8266 and ESP32, changing to DHT11 for project sensor

###Current Progress: 5/18/2019
Project is functional. Rev A is retired.

## esp32-grow
### This was an esp32 based automatic plant monitor system.
This system was designed to monitor up to 50 plants at a time and upload the data to google firebase.
The device measures 50 capacitive soil sensors, air temperature, humidity and a single pH probe.


![esp32-grow](https://github.com/Elipsit/esp-grow/blob/master/Rev%20B/pics/esp32-grow_front.png)

![esp32-grow](https://github.com/Elipsit/esp-grow/blob/master/Rev%20B/pics/esp32-grow_back.png)
