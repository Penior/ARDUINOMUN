#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <time.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27는 일반적인 I2C 주소입니다. 사용하는 LCD에 따라 다를 수 있습니다.
RTC_DS3231 rtc;

void setup() {
  lcd.clear();
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  lcd.print("Time: ");

  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loop() {
  DateTime now = rtc.now();
  lcd.setCursor(6, 0);
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);

  delay(1000);
}
