#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define PHOTO A0
DHT dht(12, DHT11);

LiquidCrystal_I2C lcd(0x27, 16, 2);
int Photo_Value = 0;
int h = 0;  // 습도 변수 추가
int t = 0;  // 온도 변수 추가

void request_dht11() {
  h = dht.readHumidity();
  t = dht.readTemperature();
}

void setup() {
  lcd.clear();
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  request_dht11();
  delay(1000);

  Photo_Value = analogRead(PHOTO);
  float pv_value = float(Photo_Value * 5) / 1023;
  float Rp = (10 * pv_value) / (5 - pv_value);
  float y = (log10(200 / Rp)) / 0.7;
  float Ea = pow(10, y);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Pa=");
  lcd.print(Photo_Value);

  lcd.setCursor(0, 1);
  lcd.print(" Lx=");
  lcd.print(Ea);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print(" Humidity=");
  lcd.print(h);

  lcd.setCursor(0, 1);  
  lcd.print(" Temperature=");
  lcd.print(t);

  delay(1000);
}
