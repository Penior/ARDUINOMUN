/* DS3231 UnixTime demo

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
}

void loop() {
  // The UnixTime of past
  DateTime Someday;
  Someday.year = 2009;
  Someday.month = 2;
  Someday.day = 13;
  Someday.hour = 23;
  Someday.minute = 31;
  Someday.second = 30;
  long UnixTime = rtc.getUnixTime(Someday);

  // Print past UnixTime
  Serial.print(Someday.year);   Serial.print('.');
  Serial.print(Someday.month);  Serial.print('.');
  Serial.print(Someday.day);    Serial.print(' ');
  Serial.print(Someday.hour);   Serial.print(':');
  Serial.print(Someday.month);  Serial.print(':');
  Serial.print(Someday.second); Serial.print(" = ");
  Serial.println(UnixTime);

  // The UnixTime of now
  DateTime now = rtc.getDateTime();
  UnixTime = rtc.getUnixTime(now);

  // Print current UnixTime
  Serial.print(now.year);   Serial.print('.');
  Serial.print(now.month);  Serial.print('.');
  Serial.print(now.day);    Serial.print(' ');
  Serial.print(now.hour);   Serial.print(':');
  Serial.print(now.month);  Serial.print(':');
  Serial.print(now.second); Serial.print(" = ");
  Serial.println(UnixTime);
  Serial.println();
  
  // Wait one second before repeating
  delay (1000);
}
