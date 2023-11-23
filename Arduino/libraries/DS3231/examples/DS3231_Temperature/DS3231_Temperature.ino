/* DS3231: How to getting temperature

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

// Init the DS3231 using the hardware interface
DS3231 rtc(SDA, SCL);

void setup() {
  // Setup Serial connection
  Serial.begin(9600);
  Serial.println("OK!");

  // Initialize the rtc object
  rtc.begin();
}

void loop() {
  // Print temperature
  Serial.print("Current temperature is ");
  Serial.print(rtc.getTemperature());
  Serial.println(" C");

  // Wait three seconds before repeating
  delay(3000);
}
