#include <Wire.h>
#include <time.h>
#include "RTClib.h"
// 모듈 포함 부분입니다. 위의 순서대로 라이브러리를 추가하지 않을 시에 작동이 불가합니다.

RTC_DS3231 rtc; //RTC로 선언

void setup () 
{
  Serial.begin(9600);// 시리얼 프린트 시작
  
  delay(3000); // 콘솔을 열기를 기다립니다.

  if (! rtc.begin()) { 
//(!는 논리 부정연산자로 거짓을 참으로 참을 거짓으로 바꾸어 줍니다. 즉 RTC가 시작을 하지 않는다면 
//이라는 조건입니다.)
    Serial.println("RTC를 찾을 수 없습니다.");
    while (1); //1번 반복
  }
  
  if (rtc.lostPower()) { //RTC에 전원이 없다면
    Serial.println("RTC가 전원이 없습니다. 시간을 설정합니다."); //
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //시각을 설정 날짜와 시간을 세팅합니다.
    
    // 아래와 같은 코팅을 한다면 January 27 2017 at 12:56 
    // rtc.adjust(DateTime(2017, 1, 27, 12, 56, 0)); 이렇게 설정하셔야합니다.
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

}

void loop () 
{
    DateTime now = rtc.now(); //현재시간을 세팅
    Serial.println("Current Date & Time: "); //현재 날짜와 시간
    Serial.print(now.year(), DEC); //년
    Serial.print('/');
    Serial.print(now.month(), DEC); // 월
    Serial.print('/');
    Serial.print(now.day(), DEC); // 날
    Serial.println();
    Serial.print(now.hour(), DEC); // 시간
    Serial.print(':');
    Serial.print(now.minute(), DEC); //분
    Serial.print(':');
    Serial.print(now.second(), DEC); // 초
    Serial.println();
    Serial.print(rtc.getTemperature()); //온도 표시도 가능합니다.
    Serial.println('C');

    lcd.print()
    delay(1000);
}
