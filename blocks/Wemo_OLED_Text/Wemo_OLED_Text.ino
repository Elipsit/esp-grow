#include <Wire.h>  // Include Wire if you're using I2C
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 1  // Wemo SCL 
MicroOLED oled(PIN_RESET, 0x3C);    // I2C declaration

void setup()
{
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  delay(1000);     // Delay 1000 ms
  oled.clear(PAGE); // Clear the buffer.
  
}

void loop() {
// Demonstrate font 1. 5x8. Let's use the print function
  // to display every character defined in this font.
  oled.setFontType(0);  // Set font to type 1
  oled.clear(PAGE);     // Clear the page
  oled.setCursor(8, 0); // Set cursor to top-left
  // Print can be used to print a string to the screen:
  oled.print("Wifi Test");
  oled.display();       // Refresh the display
  delay(1000);          // Delay a second and repeat
    oled.setCursor(10, 20);
  oled.print("Conneting..");
  oled.display();
  delay(1000);

}

// Center and print a small title
// This function is quick and dirty. Only works for titles one
// line long.
void printTitle(String title, int font)
{
  int middleX = oled.getLCDWidth() / 2;
  int middleY = oled.getLCDHeight() / 2;
  
  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the middle of the screen
  oled.setCursor(middleX - (oled.getFontWidth() * (title.length()/2)),
                 middleY - (oled.getFontWidth() / 2));
  // Print the title:
  oled.print(title);
  oled.display();
  delay(1500);
  oled.clear(PAGE);
}

