#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <string.h>



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


void time_setup(){
  char* time = __TIME__;
  char* date = __DATE__;
  char m[3] = {time[0], time[1], time[2]};
  char* mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  uint16_t year=0;
  uint8_t month=0, day=0, hour=0, minute=0, second=0;
  year = ((int)date[7]-48)*1000+((int)date[8]-48)*100+((int)date[9]-48)*10+((int)date[10]-48);
  day = ((int)date[4]-48)*10+((int)date[5]-48);
  hour = ((int)time[0]-48)*10+((int)time[1]-48);
  minute = ((int)time[3]-48)*10+((int)time[4]-48);
  second = ((int)time[6]-48)*10+((int)time[7]-48);
  for (int i=0; i<12; i++){
    if (m == mon[i]){
      month = i+1;
    }
  }
  Clock.setDate(year, month, day);
  Clock.setTime(hour, minute, second);
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
 resetTimeLCD(1000);
}
