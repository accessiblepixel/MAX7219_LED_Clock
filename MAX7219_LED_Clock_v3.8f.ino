/*   FILENAME: MAX7219_LED_Clock_with_dbg.ino
 *   VERSION: 3.8f
 *   AUTHOR(s): Jessica at AccessiblePixel.com
 *   CONTACT: media@accessiblepixel.com
 *   LICENSE: 
 *     
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//
// Include Libraries
//

#include <avr/wdt.h> // Arduino Watchdog Library
#include <Time.h>
#include <TimeLib.h>  // Time Manipulation Library
#include <DS1307RTC.h>  // DS1307 RTC Library
#include "LedControl.h" // MAX72XX Library


//
// Defines
//

#define TOTAL_STRIPS 2 // Set How Many Strips of MAX7219 7 Segment Displays
#define DATAIN 9 // pin 9 is connected to the DataIn 
#define CLK 8 // pin 8 is connected to the CLK
#define LOAD 7 // pin 7 is connected to LOAD 
#define DIGITS 2 // We have 8 digits on our MAX7219
#define debugPin A0 // Debug Pin. Bring low to enable debugging with serial.
#define debugLED 13 // Debug LED
#define speakerPin 6 // Speaker/Piezo Pin


//
// Start LedControl MAX7219 Library
//

LedControl lc=LedControl(DATAIN,CLK,LOAD,TOTAL_STRIPS);

//
// Set Variables
//

int year4digit;  // 4 digit year
byte c = 1;
char k;

//
// Debug Mode
// Will be enabled if A0 is brought LOW
//

bool isDebug() {
  return digitalRead((unsigned char) debugPin) == LOW ? true : false;
}

//
// Debug Function Declaration
//

template<typename T>
void debugPrint(T printMe, bool newLine = false) {
  if (isDebug()) {
    if (newLine) {
      Serial.println(printMe);
    }
    else {
      Serial.print(printMe);
    }
    Serial.flush();
  }
}
void setupFinished() {
  for (k = 1; k <= 3; k = k + 1) {
      digitalWrite(debugLED, HIGH);
      delay(150);
      digitalWrite(debugLED, LOW);
      delay(150);
  }
  if (digitalRead(debugPin) == LOW) {
    digitalWrite(debugLED, HIGH);
  }
}

void initialize_display(bool firstboot = false) {
  if(isDebug()) {
    debugPrint("Initializing Display",true);
  }
  for (int loop = 0; loop < (TOTAL_STRIPS); loop++) {
    lc.shutdown(loop,true);
      if (isDebug()) {
        debugPrint("lc.shutdown,true - ",false);
        debugPrint(loop,true);
      }
    lc.clearDisplay(loop);
      if (isDebug()) {
        debugPrint("lc.clearDisplay - ",false);
        debugPrint(loop,true);
      }
    delay(250);
    lc.shutdown(loop,false);
      if (isDebug()) {
        debugPrint("lc.shutdown,false - ",false);
        debugPrint(loop,true);
      }
    // Set the brightness to a medium value
    lc.setIntensity(loop,0);
      if (isDebug()) { 
        debugPrint("lc.setIntensity,4 - ",false);
        debugPrint(loop,true);
      }
    // Clear the display
    if(firstboot) {
      lc.clearDisplay(loop);
      }
    if (isDebug()) { 
      debugPrint("lc.clearDisplay - ",false);
      debugPrint(loop,true);
    }
    
  }
  if(firstboot && isDebug()) {
    tone(speakerPin, 1760, 100);
    delay(150);
    tone(speakerPin, 1760, 100);
    delay(100);
    noTone(speakerPin);
  int devices=lc.getDeviceCount();
    for(int row=8;row>=0;row--) {       delay(15);wdt_reset();
      for(int col=8;col>=0;col--) {
        for(int address=0;address<devices;address++) {//delay(150);wdt_reset();
          lc.setLed(address,row,col,true);
          delay(7);
          wdt_reset();
          lc.setLed(address,row,col,false);
        } 
      }
    }
  }
  wdt_reset();
}

//
// Setup (Run Once)
//

void setup() {

  Serial.begin(9600); // Start Serial for Debugging
  pinMode((unsigned char) debugPin, INPUT_PULLUP); // Set pin Analog 0 to an INPUT_PULLUP
  pinMode(debugLED, OUTPUT); // Set debugLED to OUTPUT

//
// Enable Arduino Watchdog
// Enable to run every 2 seconds and reset the Arduino if something goes wrong.
//

  wdt_enable(WDTO_4S);
    if (isDebug()) { 
          debugPrint("Watchdog Enabled",true);
    }

//
// The MAX7219 needs waking up as it is in power-saving mode on startup.
// This will wake all connected displays.
//
  initialize_display(true);
  setupFinished(); 
}

//
// Loop (Run Forever)
//

void loop() {
  if(!isDebug()) {
    digitalWrite(debugLED,LOW);
  }
  if(isDebug()) {
    digitalWrite(debugLED,HIGH);
  }

//
// Get data from the DS1307 RTC
//

  tmElements_t tm;
    if (RTC.read(tm)) {
        year4digit = tm.Year + 1970;  // Create an accurate 4 digit year variable
    } 
    else { 
      if (isDebug()) { 
        debugPrint("Error communicating with RTC. Forcing reset (via watchdog) ...",true);
        }
      delay (5000);   // Force a Reset with the Arduino Watchdog
        if (isDebug()) { 
          debugPrint("Watchdog Failed",true);
        }
     }
  
//
// Get Single Digits from each number using modulus
//
  
// Quick Guide to Modulus with Arduino
// Given the number 2018:
// How to get:
// 8 is 2018 % 10
// 1 is 2018 / 10 % 10
// 0 is 2018 / 100 % 10
// 2 is 2018 / 1000 % 10

  int firsthourdigit = tm.Hour /10 % 10;
  int secondhourdigit = tm.Hour % 10;
  int firstminutedigit = tm.Minute /10 % 10;
  int secondminutedigit = tm.Minute % 10;
  int firstseconddigit = tm.Second /10 % 10;
  int secondseconddigit = tm.Second % 10;
  int firstdigitday = tm.Day /10 % 10;
  int seconddigitday = tm.Day %10;
  int firstdigitmonth = tm.Month /10 % 10;
  int seconddigitmonth = tm.Month % 10;
  int firstdigityear = year4digit / 1000 % 10;
  int seconddigityear = year4digit / 100 % 10;
  int thirddigityear = year4digit / 10 % 10;
  int fourthdigityear = year4digit %10;

//
// Output the digits
//
    
//
// First Display (as wired) displaying Time: HH.MM.SS format
//
  //lc.setRow(0,6,B10000001);
  lc.setRow(0,7,B00001111);                 // Preceding t before the time.
  lc.setDigit(0,5,firsthourdigit,false);    // First Hour Digit     2
  lc.setDigit(0,4,secondhourdigit,true);    // Second Hour Digit    1 (with a .)
  lc.setDigit(0,3,firstminutedigit,false);  // First Minute Digit   2
  lc.setDigit(0,2,secondminutedigit,true);  // Second Minute Digit  3 (with a .)
  lc.setDigit(0,1,firstseconddigit,false);  // First Second Digit   3
  lc.setDigit(0,0,secondseconddigit,true);  // Second Second Digit  4 (with a .)

//
// Second Display (as wired) displaying Date: DD.MM.YYYY format.
//
  
  lc.setDigit(1,7,firstdigitday,false);     // First Day Digit      1
  lc.setDigit(1,6,seconddigitday,true);     // Second Day Digit     8 (with a .)
  lc.setDigit(1,5,firstdigitmonth,false);   // First Month Digit    1
  lc.setDigit(1,4,seconddigitmonth,true);   // Second Month Digit   2 (with a .)
  lc.setDigit(1,3,firstdigityear,false);    // First Year Digit     2
  lc.setDigit(1,2,seconddigityear,false);   // Second Year Digit    0
  lc.setDigit(1,1,thirddigityear,false);    // Third Year Digit     1
  lc.setDigit(1,0,fourthdigityear,false);   // Fourth Year Digit    8

  delay(50);
  
//
// Reset Watchdog
// 
  wdt_reset();

//
// Beep on the hour.
//

  if(tm.Minute == 0) {
    if (c == 0) {
      tone(speakerPin, 1760, 400);
        if (isDebug()) { 
          debugPrint("The hourly beep played. Beep disarmed.",true);
        }
      initialize_display(); 
      c++;          
    }  
    wdt_reset(); 
  }

//
// Arm the hourly beep
//

  if(tm.Minute == 59) {
    if(tm.Second == 59) {
      if(c != 0) {
        if(isDebug()) {
          debugPrint("The hourly beep is armed.", true); 
        }
        c = 0;
      }
    }
  }

//
// 1 click at 15 minutes past the hour.
//

  if(tm.Minute == 15) {
    if (c == 0) {
      tone(speakerPin, 220, 10);
        if (isDebug()) { 
          debugPrint("The 15min click played. Click disarmed.",true);
        }
      wdt_reset();
      c++;
    }   
  }

//
// Arm the 15 minute click.
//
    
  if(tm.Minute == 14) {
    if(tm.Second == 59) {
      if(c != 0) {
        if(isDebug()) { 
          debugPrint("The 15min click is armed", true); 
        }
        c = 0;
      }
    }
  }

//
// 2 clicks at 30 minutes past the hour.
//

  if(tm.Minute == 30) {
    if (c == 0) {
      tone(speakerPin, 220, 10);
      delay(100);
      tone(speakerPin, 220, 10);
      delay(100);
      noTone(speakerPin);
        if (isDebug()) { 
          debugPrint("The 30min click played. Click disarmed.",true);
        }
      wdt_reset(); 
      initialize_display(); 
      c++;
    }
  }    

//
// Arm the 30 minute clicks.
//

  if(tm.Minute == 29) {
    if(tm.Second == 59) {
      if(c != 0) {
        if(isDebug()) { 
          debugPrint("The 30min click is armed.", true);
        }
        c = 0;
      }
    }
  }

//
// 3 clicks at 45 minutes past the hour.
//

  if(tm.Minute == 45) {
    if (c == 0) {
      tone(speakerPin, 220, 10);
      delay(100);
      tone(speakerPin, 220, 10);
      delay(100);
      tone(speakerPin, 220, 10);
      delay(100);
      noTone(speakerPin);
      c++; 
      if (isDebug()) { 
        debugPrint("The 45min click played. Click disarmed.",true);
      }
    }
    wdt_reset();
  }

//
// Arm the 45 minute clicks.
//

  if(tm.Minute == 44) {
    if(tm.Second == 59) {
      if(c != 0) {
        if(isDebug()) { 
          debugPrint("The 45min click is armed.", true); 
        }
        c = 0;
      }
    }
  }

//
// End of loop()
//

}
