/* DS3231 Basic demo

  This example code describes
      sendSystemTime(),
      getDOWStamp(), getDateStamp(),
      getMonthStamp(), getTimeStamp()

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
DS3231 rtc(SDA, SCL); // Init the DS3231 using the hardware interface

void setup() {
  // Setup Serial connection
  Serial.begin(9600);
  Serial.println("OK!");

  // Initialize the rtc object
  rtc.begin();

  // Set RTC's time to PC's Sysytem at compile.
  // This code must be deleted after uploading the sketch!
  rtc.sendSystemTime();
  // Also can be set the date and time by manually
  //rtc.setDate(2020, 3, 1);  // Set the date to March 1st, 2020
  //rtc.setTime(12, 30, 0);   // Set the time to 12:30:00 (24hr format)
}

void loop() {
  // Print day of week
  Serial.print(rtc.getDOWStamp());
  Serial.print(" ");
  // Print date
  Serial.print(rtc.getDateStamp());
  Serial.print(" ");
  // Print month
  Serial.print(rtc.getMonthStamp());
  Serial.print(" ");
  // Print time
  Serial.println(rtc.getTimeStamp());
  // Wait one second before repeating
  delay (1000);
}
