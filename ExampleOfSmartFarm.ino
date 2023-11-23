#include <Servo.h>
#include "DHT.h"
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "DS3231.h"

//========================================================
// Setting For Program
#define USE_BLUETOOTH     1
#define USE_SCHEDULE      0
#define USE_SCHEDULE_CONT 1
#define USE_CONDITION     0
#define DEBUG             0
//========================================================

//========================================================
// Setting For GPIO PIN
#define DHTPIN          12
#define DHTTYPE         DHT11
#define SERVOPIN        9
#define LIGHTPIN        4
#define FAN_PIN         32
#define WATER_PUMP_PIN  31
#define RBG_R           4 
#define RBG_G           35 
#define RBG_B           36 
//========================================================

// Sensor Variable
DHT dht(DHTPIN, DHTTYPE);
Servo servo; 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Co2 Sensor - add code - Lee
extern void RX9QR_loop();
extern unsigned int co2_ppm;

//========================================================
// unsigned Lux; // add code - Lee
unsigned local_time = 0; // add code - Lee
unsigned int timeout = 0; 
unsigned int cdcValue = 0;
bool water_State = false ;// add code - Lee
unsigned water_Time = 0 ;// add code - Lee
int temperature, humidity;

//================================
// RTC Schedule //add code - Lee 
int angle;
bool Century,h12,PM;
int gYear, gMonth, date, DoW, gHour, gMinute, gSecond;
int prehour = -1;
DS3231 Clock;

 //  Scheduling [DO] Function
#define SCH_DO_LIGHT_CH   1
#define SCH_DO_WPUMP_CH   2
#define SCH_DO_SMOTOR_CH  4
#define SCH_DO_PAN_CH     8

#define MAX_DO    4

typedef struct Scheduling {
  int iTime;
  int iDoing;
  int iParameter[4];
}SCHEDULE;

#define MAX_SHEDULE 24

SCHEDULE Set_Shedule[MAX_SHEDULE] = {
  {1, 15, {0, 0, 0, 0}},   
  {2, 15, {0, 0, 0, 0}},
  {3, 15, {0, 0, 0, 0}},    
  {4, 15, {0, 0, 0, 0}},
  {5, 15, {0, 0, 0, 01}}, 
  {6, 15, {0, 0, 0, 0}},    
  {7, 15, {0, 0, 0, 0}},
  {8, 15, {0, 0, 0, 0}},    
  {9, 15, {10, 1, 1, 1}},
  {10, 15, {10, 0, 0, 0}},
  {11, 15, {10, 1, 1, 1}},   
  {12, 15, {10, 0, 0, 0}},
  {13, 15, {10, 1, 1, 1}},    
  {14, 15, {10, 0, 0, 0}},
  {15, 15, {10, 1, 1, 1}},
  {16, 15, {10, 0, 0, 0}},    
  {17, 15, {10, 1, 1, 1}},
  {18, 15, {0, 0, 0, 0}},    
  {19, 15, {0, 0, 0, 0}},
  {20, 15, {0, 0, 0, 0}},
  {21, 15, {0, 0, 0, 0}},    
  {22, 15, {0, 0, 0, 0}},
  {23, 15, {0, 0, 0, 0}},    
  {24, 15, {0, 0, 0, 0}} 
};

void DoLight1(int iPara){
  analogWrite(LIGHTPIN, (int)(25 * iPara));
}

void DoWPump1(int iPara){
  if(iPara == 0) 
    digitalWrite(WATER_PUMP_PIN, 0);
  //else digitalWrite(WATER_PUMP_PIN, 1);
  else {
    digitalWrite(WATER_PUMP_PIN, 1);
    water_State = true;
    }
           
}

void DoSMoter1(int iPara){
 if(iPara == 0) 
  angle = 10;
 else 
  angle = 80;
 servo.write(angle);
 //delay(200);  
}

void DoPan1(int iPara){
  if(iPara == 0) 
  digitalWrite(FAN_PIN, 0);
  else 
  digitalWrite(FAN_PIN, 1); 
}

int SearchingAction(){
  int i;
  // gDefaultShedule가 24개의 array
  for(i=0;i<MAX_SHEDULE;i++){
    if (gHour == Set_Shedule[i].iTime)
      return i;
  }
    return -1;
}

void DoAction(int iDo, int iPara[]){
  int i=0;
  int iIndex;
  for(i=0;i<MAX_DO;i++)
  { 
   // iIndex = iDo & 2 ^ i; 
    iIndex = iDo & (1<<i); 
    
    switch(iIndex)
    {
      case SCH_DO_LIGHT_CH:
        DoLight1(iPara[i]);
        break;
  
      case SCH_DO_WPUMP_CH:
        DoWPump1(iPara[i]);
        break;
        
      case SCH_DO_SMOTOR_CH:
        DoSMoter1(iPara[i]);
        break;
  
      case SCH_DO_PAN_CH:
        DoPan1(iPara[i]);
        break;
        
      default:
        break;
    }
  }
}


void ReadDS3231(){ 
  gSecond=Clock.getSecond();
  gMinute=Clock.getMinute();
  gHour=Clock.getHour(h12,PM);
  DoW=Clock.getDoW();
  date=Clock.getDate();
  gMonth=Clock.getMonth(Century);
  gYear=Clock.getYear();
  
 }

 
//  Scheduling 
#define MAX_DO      4
#define MAX_SCHEDULE   24
#define STEP_LIGHT    25
#define DEFALT_LIGHT_VAL 10
#define MAX_TIMEOUT 50 // LCD 프린팅 주기

unsigned Lux; // add code - Lee   


//========================================================

//========================================================
//  Server Data 
#define MIN_WIFI_DATA   11
#define BUT_AUTO        2
#define BUT_LIGHT_AUTO  20

String wifiReceivedData;
int mLightConditionHigh = 3000;
int mTempConditionHigh = 30;
int mWaterConditionHigh = 30;
int mLightConditionLow = 1000;
int mTempConditionLow = 20;
int mWaterConditionLow = 30;

int mConditionData[16] =
            {
              3000, 0, 0, 1,  // 조명
              300, 0, 1, 1,  // 습도
              3000, 0, 2, 1,  // 온도
              3000, 1, 3, 1,  // 수분              
            };



int mSetLight = -1;
int mSetPan = -1;
int mSetServo = -1;
int mSetWPump = -1;

int mActLight = -1;
int mActPan = -1;
int mActServo = -1;
int mActWPump = -1;

int mSetCurLight;
int mSetCurPan;
int mSetCurServo;
int mSetCurWPump;

int mCDCVal =-1;
int mWaterVal = -1;
int mTempVal = -1;
int mHumiVal = -1;

int mServerSchedule[MAX_SCHEDULE][5]; 
//========================================================

void DoLightAuto()
{
    int iLight = 0;

    if (mCDCVal == -1)
        iLight =  DEFALT_LIGHT_VAL * STEP_LIGHT;
    else
    {
        iLight =  mActLight;

        // ConditionData
        //      0 : 기준값
        //      1 : 이상(0), 미만(1)
        //      3 : OFF(0), ON(1)  
        if (mConditionData[1] == 0)
        {
            if (mCDCVal >= mConditionData[0])
            {
                if (mConditionData[3] == 1)
                    iLight = DEFALT_LIGHT_VAL * STEP_LIGHT;
                else
                    iLight = 0;
            }
        }
        else
        {
            if (mCDCVal < mConditionData[0])
            {
                if (mConditionData[3] == 1)
                    iLight = DEFALT_LIGHT_VAL * STEP_LIGHT;
                else
                    iLight = 0;         
            }        
        }
    } 

    mActLight = iLight;
    analogWrite(LIGHTPIN, (int)(iLight));   
}

void DoLight(int iPara){
  int iLight = 0;

  iLight = iPara;

  Serial.print("Light ");
  Serial.println(iLight);

  mActLight = iLight;
  //analogWrite(LIGHTPIN, (int)(iLight);
  analogWrite(LIGHTPIN, (int)(iLight*25));
}

void DoWPumpAuto() 
{
    int iWPump = 0;

    if (mWaterVal == -1)
      iWPump = 0;
    else
    {
      iWPump =  mActWPump; 
      if (mConditionData[13] == 0)
      {
        if (mWaterVal >= mConditionData[12])
        {
          if (mConditionData[15] == 1)
            iWPump = 1;
          else
            iWPump = 0;
        }
      }
      else
      {
        if (mWaterVal <= mConditionData[12])
        {
           if (mConditionData[15] == 1)
            iWPump = 1;
          else
            iWPump = 0;         
        }        
      }
//      if (mWaterVal > mWaterConditionHigh)
//        iWPump = 0;
//      else if (mWaterVal < mWaterConditionLow)
//        iWPump = 1; 
//      else
//        iWPump =  mActWPump; 
    }

    mActWPump = iWPump;
  
  digitalWrite(WATER_PUMP_PIN, iWPump);
}

void DoWPump(int iPara){
  int iWPump = 0;
  
  iWPump = iPara;  

    Serial.print("WPump ");
  Serial.println(iWPump);
  
  mActWPump = iWPump;
  
  digitalWrite(WATER_PUMP_PIN, iWPump);
}

void DoSMoterAuto()
{
      int iMoter = 0;
    if (mTempVal == -1)
      iMoter = 0;
    else
    {
      iMoter =  mActServo; 
      if (mConditionData[9] == 0)
      {
        if (mTempVal >= mConditionData[8])
        {
          if (mConditionData[11] == 1)
            iMoter = 80;
          else
            iMoter = 0;
        }
      }
      else
      {
        if (mWaterVal <= mConditionData[8])
        {
           if (mConditionData[11] == 1)
            iMoter = 80;
          else
            iMoter = 0;         
        }        
      }
    }

  mActServo = iMoter;
  servo.attach(SERVOPIN);
  servo.write(iMoter);
}

void DoSMoter(int iPara){
  int iMoter = 0;
  
  if (iPara == 0)
  {
    iMoter = 0;  
  }
  else
  {
    iMoter = 80;
  }
    Serial.print("SMoter ");
  Serial.println(iMoter);
  
  
  mActServo = iMoter;
  servo.attach(SERVOPIN);
  servo.write(iMoter);
}

void DoPanAuto()
{
      int iPan = 0;
    if (mTempVal == -1)
      iPan = 0;
    else
    {
      iPan =  mActPan;
      if (mConditionData[5] == 0)
      {
        if (mTempVal >= mConditionData[4])
        {
          if (mConditionData[7] == 1)
            iPan = 1;
          else
            iPan = 0;;
        }
      }
      else
      {
        if (mWaterVal <= mConditionData[4])
        {
          if (mConditionData[7] == 1)
            iPan = 1;
          else
            iPan = 0;;      
        }        
      }
      //if (mTempVal > mTempConditionHigh)
      //  iPan = 0;
      //else if (mTempVal < mTempConditionLow)
      //  iPan = 1; 
      //else
      //  iPan =  mActPan; 
    }

  mActPan = iPan;

  digitalWrite(FAN_PIN, iPan);
}

void DoPan(int iPara){
  int iPan;
  
  iPan = iPara;

  mActPan = iPan;

      Serial.print("Pan ");
  Serial.println(iPan);

  digitalWrite(FAN_PIN, iPan);
}

void DoCondition() {
  if (mSetLight == BUT_LIGHT_AUTO)
    DoLightAuto();
  if (mSetWPump == BUT_AUTO)
    DoWPumpAuto();
  if (mSetServo == BUT_AUTO)
    DoSMoterAuto();
  if (mSetPan == BUT_AUTO)
    DoPanAuto();
}


#if 0
void ChangeActByCondition(int temp, int cdcval)
{
  if (mLightConditionEnable == 1)
  {
    if (cdcval < mLightConditionLow)
      DoLight(mActLight);
    else if (cdcval > mLightConditionHigh)
      DoLight(0);    
  }

  if (mTempConditionEnable == 1)
  {
    if (temp < mTempConditionLow)
      DoSMoter(0);
    else if (temp > mTempConditionHigh)
      DoSMoter(1);    
  }
}
#endif

void ChangeCondition()
{
    if (mSetWPump == BUT_AUTO)
        DoWPumpAuto();    
    if (mSetPan == BUT_AUTO)
        DoPanAuto();
    if (mSetServo == BUT_AUTO)
        DoSMoterAuto();
    if (mSetLight == BUT_LIGHT_AUTO)
        DoLightAuto();
}

void ChangeActByKey()
{
  if (mSetWPump != mSetCurWPump)
  {
    mSetWPump = mSetCurWPump;

    if (mSetWPump == BUT_AUTO)
        DoWPumpAuto();
    else
        DoWPump(mSetWPump);
  }

  if (mSetPan != mSetCurPan)
  {
    mSetPan = mSetCurPan;

    if (mSetPan == BUT_AUTO)
        DoPanAuto();
    else
        DoPan(mSetPan);
  }   

  if (mSetServo != mSetCurServo)
  {
    mSetServo = mSetCurServo;
    
    if (mSetServo == BUT_AUTO)
        DoSMoterAuto();
    else   
        DoSMoter(mSetServo);
  } 

  if (mSetLight != mSetCurLight)
  {
    mSetLight = mSetCurLight;

    if (mSetLight == BUT_LIGHT_AUTO)
        DoLightAuto();
    else 
        DoLight(mSetLight);
  }
}

//--------------------------------------------
void printLCD(int col, int row , char *str) {
    for(int i=0 ; i < strlen(str) ; i++){
      lcd.setCursor(col+i , row);
      lcd.print(str[i]);
    }}

void setup() {

  // Pin 설정
  pinMode(LIGHTPIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(RBG_R,OUTPUT);
  pinMode(RBG_G,OUTPUT);
  pinMode(RBG_B,OUTPUT);

  // 시리얼 설정
  Serial.begin(9600);
  Serial1.begin(9600);
 
  // 온도/습도 센서 작동
  dht.begin();
  lcd.init();
  lcd.backlight();
  printLCD(0, 0, "MMB_SmartFarm");
  printLCD(0, 1, "Are you Ready? ");  
  delay(3000);
  analogWrite(LIGHTPIN, 255);
  servo.attach(SERVOPIN);
  servo.write(0);
}

void loop() { 
 //  static int displayToggle = 1;
  
  int motorval = 0;
  int get_co2_ppm = 0;  
  int waterValue = 0;
  char sData[128] = { 0x00, };
  char rData[32] = { 0x00, };
  char nData[32] = { 0x00, };
  int rPos = 0;
  int nPos = 0;
  int right = 10;
  int pt100 = random(80)-10;
  int ph = random(140);

  // put your main code here, to run repeatedly:
      cdcValue = analogRead(0);
    //Photo ADC => Lux Calibration 
    float pv_value = float(cdcValue*5)/1023;
    float Rp = (10*pv_value)/(5-pv_value);
    float y = (log10(200/Rp))/0.7; 
    float Ea = pow(10,y); 
    Lux = (int)Ea ;
    
    cdcValue = (int)Lux;
    //Serial.print(cdcValue); Serial.print(",");
    
   float waterADC = analogRead(1);
    //Serial.print(waterADC); Serial.print(",");
    waterADC = ((1023 -waterADC)*100)/1023 ;
    waterValue = (int)waterADC;
//    Serial.print(waterValue); Serial.print(",");
   
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    mCDCVal = cdcValue;
    mWaterVal = waterValue;
    mTempVal = temperature;
    mHumiVal = humidity;
    
   timeout += 1;
  
    if((timeout %50)==1) {
    lcd.clear();
    memset(sData, 0x00, 64);
    sprintf(sData, "temp %02dC humi %02d%%", temperature,humidity);
    printLCD(0, 0, sData);
   
    memset(sData, 0x00, 64);
    sprintf(sData, "%05dLx soi=%03d%%", Lux, waterValue);
    printLCD(0, 1, sData);
    }
    
     if((timeout %50)==15) {
      lcd.clear();
      memset(sData, 0x00, 64);
      sprintf(sData,"pt:%02dC ph:%d.%d",pt100,ph/10,ph%10);
      printLCD(0, 0, sData);
    
      memset(sData, 0x00, 64);
      sprintf(sData,"co2=%04dppm", co2_ppm);
      printLCD(0, 1, sData);
    }

   if((timeout %50)==30) {
      lcd.clear();
      memset(sData, 0x00, 64);
      ReadDS3231();// 시간 읽음 --> hour
     sprintf(sData,"What time is now?");
      printLCD(0, 0, sData);
     sprintf(sData,"%02d_%01d_%01d %02d:%02d:%02d",
     gYear,gMonth,date,gHour,gMinute,gSecond);
      printLCD(0, 1, sData);
    }

  

   if((timeout %50)==1) {

     memset(sData, 0x00, 128);
     sprintf(sData, "{ \"temp\":%d,\"humidity\":%d,\"cdc\":%d,\"water\":%d,\"co2\":%d, \"pt100\":%d, \"ph\":%d.%d }", 
        (int)temperature, (int)humidity,
        cdcValue, waterValue, co2_ppm, 
        pt100,
        ph / 10, ph %10
        );
        
      Serial.println(sData);
      // Bluetooth Data Sending
      Serial1.println(sData);
   }  
  // Bluetooth Control
  while(0 < Serial1.available()) 
  {
    char ch = Serial1.read();
    rData[rPos] = ch; rPos += 1;
    Serial.print(ch);

    if(ch == '\n')
    {         
#if DEBUG
      Serial.print("rPos=");
      Serial.print(rPos);
      Serial.print(" ");
      Serial.println(rData);
#endif
  
      if(memcmp(rData, "C_S-", 4) == 0)
      {
        if(rData[4] == '0') motorval = 10;
        else motorval = 80;
        servo.attach(SERVOPIN);
        servo.write(motorval); 
        delay(500);
        servo.detach();
#if DEBUG
        Serial.print("server_f_MOTOR=");
        Serial.println(motorval);
#endif
      }
  
      if(memcmp(rData, "C_F-", 4) == 0)
      {
        if(rData[4] == '0') digitalWrite(FAN_PIN, 0);
        else digitalWrite(FAN_PIN, 1);
#if DEBUG
        Serial.print("FAN=");
        Serial.println(rData[4]);
#endif
      }
  
      if(memcmp(rData, "C_L-", 4) == 0)
      {
        int light = atoi(rData+4);
        analogWrite(LIGHTPIN, (int)(25 * light));
#if DEBUG
        Serial.print("LIGHT=");
        Serial.println(25 * light); // light);
#endif
      }
  
      if(memcmp(rData, "C_W-", 4) == 0)
      {
        if(rData[4] == '0') digitalWrite(WATER_PUMP_PIN, 0);
        //else digitalWrite(WATER_PUMP_PIN, 1);
        else {
          digitalWrite(WATER_PUMP_PIN, 1);
          water_State = true;
          }
#if DEBUG
        Serial.print("WATER=");
        Serial.println(rData[4]);
#endif
      }
  
      rPos = 0;
      memset(rData, 0x00,32);
      break;
    }
    delay(10);
  }



#if USE_CONDITION
  // 조건에 따른 ....
  ChangeCondition();
  ChangeActByKey();
#endif

  RX9QR_loop(); // code add - Lee 
  
  // loop()함수 사이클 측정용도   
  local_time = millis(); 
//  Serial.print("local_time = ");
//  Serial.println(local_time);

 // water_pump timer 2~3 Minutes
  if (water_State){
    water_Time +=1; 
     if ((water_Time) > 1000){
     digitalWrite(WATER_PUMP_PIN, 0);
     water_State = false; 
     water_Time =0;
    }
  }

    // 시간 읽음 --> hour
  ReadDS3231();
 
  //시간이 바뀌는지 확인 (prehour = 9, hour = 10)
  if (prehour != gHour){
    // 정의된 스케줄 중에 그 시간이 있는지 확인
   int iTime = SearchingAction();
    // prehour를 다시 설정
   prehour = gHour;
    // 정의된 스케줄 중에 그 시간이 있을 때만 DoAction
   if(iTime != -1){
     DoAction(Set_Shedule[iTime].iDoing, 
     Set_Shedule[iTime].iParameter);   
    }
  }
  
      
  delay(100); 
}
