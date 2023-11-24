#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <time.h>
#include "RTClib.h"
#include "RX9QR.h"

#define EMF_pin 0   // RX-9 E with A0 of arduino
#define THER_pin 1  // RX-9 T with A1 of arduino
#define ADCvolt 5
#define ADCResol 1024
#define Base_line 432
#define meti 60  
#define mein 120 //Automotive: 120, Home or indoor: 1440

//CO2 calibrated number
float cal_A = 372.1; // you can take the data from RX-9 bottom side QR data #### of first 4 digits. you type the data to cal_A as ###.#
float cal_B = 63.27; // following 4 digits after cal_A is cal_B, type the data to cal_B as ##.##

//CO2 Step range
#define cr1  700      // Base_line ~ cr1
#define cr2  1000     // cr1 ~ cr2
#define cr3  2000     // cr2 ~ cr3
#define cr4  4000     // cr3 ~ cr4 and over cr4

// Thermister constant
// RX-9 have thermistor inside of sensor package. this thermistor check the temperature of sensor to compensate the data
// don't edit the number
#define C1 0.00230088
#define C2 0.000224
#define C3 0.00000002113323296
float Resist_0 = 15;

//Timing
unsigned int time_s = 0;
unsigned int time_s_prev = 0;
unsigned int time_s_set = 1;

extern volatile unsigned long timer0_millis;

//CO2 Value
int status_sensor = 0;
unsigned int co2_ppm = 0;
unsigned int co2_step = 0;
float EMF = 0;
float THER = 0;

RX9QR RX9(cal_A, cal_B, Base_line, meti, mein, cr1, cr2, cr3, cr4);

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27는 일반적인 I2C 주소입니다. 사용하는 LCD에 따라 다를 수 있습니다.
RTC_DS3231 rtc;

void setup() {

  Serial.begin(9600);
  LCD();

}

void loop() {
  LCD_setting();
  CO2();
}

void LCD() {
  lcd.clear();
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void LCD_setting() {

  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  lcd.print(now.year(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);
  lcd.print(' ');
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  delay(1000);
  lcd.setCursor(0, 1);

  if(status_sensor) {
    lcd.print(co2_ppm, DEC);
    lcd.print('p'); lcd.print('p'); lcd.print('m');
  }

}

void CO2() {
  // put your main code here, to run repeatedly:
  time_s = millis()/1000;
  if(time_s - time_s_prev >= time_s_set){
    time_s_prev = time_s;
    //every 1 sec
    //read EMF data from RX-9, RX-9 Simple START-->
    EMF = analogRead(EMF_pin);
    delay(1);
    EMF = EMF / (ADCResol - 1);
    EMF = EMF * ADCvolt;
    EMF = EMF / 6;
    EMF = EMF * 1000;
    // <-- read EMF data from RX-9, RX-9 Simple END 

    //read THER data from RX-9, RX-9 Simple START-->
    THER = analogRead(THER_pin);
    delay(1);
    THER = 1/(C1+C2*log((Resist_0*THER)/(ADCResol-THER))+C3*pow(log((Resist_0*THER)/(ADCResol-THER)),3))-273.15;
    // <-- read THER data from RX-9, RX-9 Simple END
    
    status_sensor = RX9.status_co2();   //read status_sensor, status_sensor = 0 means warming up, = 1 means stable
    co2_ppm = RX9.cal_co2(EMF,THER);    //calculation carbon dioxide gas concentration. 
    co2_step = RX9.step_co2();          //read steps of carbon dioixde gas concentration. you can edit the step range with cr1~cr4 above.
    Serial.print("# ");                //Starting letter
    if(co2_ppm <1000){
      Serial.print("0");
    }
    else{
    }

    if(status_sensor){
      Serial.print(co2_ppm); Serial.print(" ");
      Serial.print("ppm"); 
      
    }
    else{
      Serial.print("(LOADING)");
    }
    Serial.println(""); //CR LF
  }  
}

