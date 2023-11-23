/* DS3231: Demo for how to use DateTime and getDateTime()

  This example code describes
      Structure of DateTime
          (year, month, day, dow, hour, minute, second)
      Function for getDateTime()

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
DateTime now;

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
  // rtc.setDate(2020, 3, 1);  // Set the date to March 1st, 2020
  // rtc.setTime(12, 30, 0);   // Set the time to 12:30:00 (24hr format)
}

void loop() {
  // Get Date and Time information
  now = rtc.getDateTime();

  // Print day of week
  String DayOfWeek;
  switch (now.dow) {
    case MONDAY:
      DayOfWeek = "Today is Monday";
      break;
    case TUESDAY:
      DayOfWeek = "Today is Tuesday";
      break;
    case WEDNESDAY:
      DayOfWeek = "Today is Wednesday";
      break;
    case THURSDAY:
      DayOfWeek = "Today is Thursday";
      break;
    case FRIDAY:
      DayOfWeek = "Today is Friday";
      break;
    case SATURDAY:
      DayOfWeek = "Today is Saturday";
      break;
    case SUNDAY:
      DayOfWeek = "Today is Sunday";
      break;
  }
  Serial.print(DayOfWeek);
  Serial.print(", ");

  // Print date
  Serial.print(now.year);
  Serial.print("-");
  Serial.print(now.month);
  Serial.print("-");
  Serial.print(now.day);
  Serial.print(" ");

  // Print time
  Serial.print(now.hour);
  Serial.print(":");
  Serial.print(now.minute);
  Serial.print(":");
  Serial.print(now.second);
  Serial.print('\n');

  // Wait one second before repeating
  delay (1000);
}
