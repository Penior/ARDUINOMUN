/* DS3231 SWQ wave demo

  DS3231 Real-Time Clock Library
  Version: 1.0.0
  (c) 2020 Parker's Lab. All right reserved
  https://blog.naver.com/essemparker
  
  This library has been made to easily use the DS3231 RTC with an Arduino Uno.
  This is free software; you can redistribute it and/or modify it.

  To use the hardware I2C interface of the Arduino you must connect the pins as follows
      RTC DS3231      Arduino Uno
      SDA pin   -->   SDA pin or Analog 4 (A4) 
      SCL pin   -->   SCL pin or Analog 5 (A5)

  Enjoy Arduino with the DS3231 !!
*/

#include <DS3231.h>
#include "Frequency.h"
// Init the DS3231 using the hardware interface
DS3231 rtc(SDA, SCL);
int Wave = 0;

void setup() {
  // Setup Serial connection
  Serial.begin(9600);
  Serial.println("OK!");

  // Initialize the rtc object
  rtc.begin();

  // enableSQW is enable or disable
  // rtc.enableSQW(false);

  if (rtc.isSQWMode()) {
    Serial.print("SQW mode is running (");
    String Hz;
    switch (rtc.getSQWRate()) {
      case SQW_1Hz:
        Hz = "1Hz";
        break;
      case SQW_1kHz:
        Hz = "1kHz";
        break;
      case SQW_4kHz:
        Hz = "4kHz";
        break;
      case SQW_8kHz:
        Hz = "8kHz";
        break;
    }
    Serial.print(Hz); Serial.println(")");
  } else {
    Serial.println("SQW mode is disabled.\n  Now starting!");
    rtc.enableSQW(true);
  }

  // Checking for Oscillating
  pinMode(interruptPin, INPUT_PULLUP);
  prevMicros = micros();
  attachInterrupt(digitalPinToInterrupt(interruptPin), Measure, FALLING);
}

void loop() {
  unsigned long currMicros = micros();
  if (currMicros - prevMicros >= OneSecond) {
    Serial.print(Frequency);
    Serial.println("Hz");
    Wave++;
    if (Wave > SQW_8kHz) Wave = SQW_1Hz;
    rtc.setSQWRate(Wave);
    Frequency = 0;
    prevMicros = micros();
  }
}
