#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <string.h>
#include <time.h>



LiquidCrystal_I2C lcd_set(0x27,20,4);
LiquidCrystal_I2C lcd_temp(0x27,20,4);
LiquidCrystal_I2C lcd_time(0x27,20,4);
DS3231 Clock(SDA,SCL);


void lcd_setup(){
  lcd_set.begin(16,2);
  lcd_temp.begin(16,2);
  lcd_time.begin(16,2);
  lcd_set.backlight();
  lcd_set.setCursor(0,0);
  lcd_set.print("Setup Complete!");
}

void time_setup(){ // 현재 시간 얻기 불가 
  std::time_t currentTime = std::time(NULL);
    std::tm* localTime = std::localtime(&currentTime);

    int y = localTime->tm_year + 1900; // 년
    int m = localTime->tm_mon + 1;    // 월 (0부터 시작하므로 1을 더함)
    int d = localTime->tm_mday;         // 일
    int h = localTime->tm_hour;        // 시
    int min = localTime->tm_min;       // 분
    int sec = localTime->tm_sec;
  Clock.begin();
  Clock.setTime(h, min, sec);
  Clock.setDate(y, m, d);
}

void setup()
{
  lcd_setup();
  time_setup();
  delay(2000);
}

void resetTimeLCD(int ms){
  String year, month, day, hour, minute, second;
  year = String(Clock.getDateTime().year);
  month = String(Clock.getDateTime().month);
  day = String(Clock.getDateTime().day);
  hour = String(Clock.getDateTime().hour);
  minute = String(Clock.getDateTime().minute);
  second = String(Clock.getDateTime().second);
  lcd_time.clear();
  lcd_time.backlight();
  lcd_time.setCursor(3,0);
  lcd_time.print(year);
  lcd_time.setCursor(7,0);
  lcd_time.print(".");
  lcd_time.setCursor(8,0);
  lcd_time.print(month);
  lcd_time.setCursor(10,0);
  lcd_time.print(".");
  lcd_time.setCursor(11,0);
  lcd_time.print(day);
  lcd_time.setCursor(4,1);
  lcd_time.print(hour);
  lcd_time.setCursor(6,1);
  lcd_time.print(":");
  lcd_time.setCursor(7,1);
  lcd_time.print(minute);
  lcd_time.setCursor(9,1);
  lcd_time.print(":");
  lcd_time.setCursor(10,1);
  lcd_time.print(second);
  delay(ms);
}


void loop()
{
  /*
  lcd_temp.clear();
  lcd_temp.backlight();
  lcd_temp.setCursor(0,0);
  lcd_temp.print("2-6");
  lcd_temp.setCursor(0,1);
  lcd_temp.print("Not defined");
  delay(2000);
*/
  testtime(1000);
}
