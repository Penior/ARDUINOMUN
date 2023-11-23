/* DS3231 SQ32kHz demo

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

#include "Frequency.h"
#include <DS3231.h>
DS3231 rtc(SDA, SCL); // Init the DS3231 using the hardware interface

void setup() {
  // Setup Serial connection
  Serial.begin(9600);
  Serial.println("OK!");

  // Initialize the rtc object
  rtc.begin();
  
  // enable32kHz is enabled or disabled
  // rtc.enable32kHz(false);

  if (rtc.is32kHzOutput()) {
    Serial.println("32kHz Output is enabled");
  } else {
    Serial.println("32kHz Output is disabled.\nNow starting!");
    rtc.output32kHz(true);
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
    Frequency = 0;
    prevMicros = micros();
  }
}
