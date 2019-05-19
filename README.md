# esp-grow
This project contains the Altium Designer schematic and pcb files, wiring diacrams and code for an esp8266 based indoor plant monitoring system.

This system is comprised of three main blocks:
1) Wemos D1 - ESP-12F; I really like working with this form factor since it has an esp8266 with a breadboard header and a CH340G
2) ESP-Grow_OLED_Rev_A: custom 2 layer pcb with a generic SSD1306 128x64 I2C monochrome OLED screen and a tack button seated under the screen.      Basically a screen button.
3) ESP-Grow_OLED_Main_A: The motherboard. A custom 4 Layer pcb with all the connectors, power rails and leds.

Update:
-DHT22 library has an error with ESP8266 and ESP32, changing to DHT11 for project sensor

Current Progress: 5/18/2019
Main and OLED pcbs have been sent to fab.

