#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define PHOTO A0

LiquidCrystal_I2C lcd(0x27, 16, 2);
int Photo_Value =0; 

void setup() {
  lcd.clear();   
  lcd.init();  
  lcd.backlight(); 
 }

void loop() {
  Photo_Value = analogRead(PHOTO);
  float pv_value = float(Photo_Value*5)/1023;
  float Rp = (10*pv_value)/(5-pv_value);
  float y = (log10(200/Rp))/0.7; 
  float Ea = pow(10,y); 

  lcd.clear();  
  lcd.setCursor(0, 0);
  lcd.print(" Pa=");
  lcd.print(Photo_Value);
  
  lcd.setCursor(0, 1);
  lcd.print(" Lx=");
  lcd.print(Ea);
  delay(1000);
  }

  
