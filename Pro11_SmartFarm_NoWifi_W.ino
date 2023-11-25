// Library Include 
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// MMB BD HW Pin Assgnment 
#define LIGHTPIN 4
#define SERVOPIN 9
#define DHTTYPE DHT11
#define DHTPIN 12
#define WATER_PUMP_PIN 31
#define FAN_PIN 32
int RBG_R = 23; 
int RBG_G = 35; 
int RBG_B = 36; 

#define DEBUG 1


// Global variable (전역변수 정의)
int angle = 0;
int get_co2_ppm = 0; 
float temperature, humidity; 
int cdcValue = 0;
int waterValue = 0;
int lightOutput = 0;
int fanOutput = 0;
int waterPumpPin = 0;
int timeout = 0; 
bool water_State = false ;// add code - Lee
unsigned water_Time = 0 ;// add code - Lee
unsigned local_time = 0; // add code - Lee

char sData[64] = { 0x00, };
char rData[32] = { 0x00, };
char nData[32] = { 0x00, };
int rPos = 0;
int nPos = 0;
int right = 10;
int displayToggle = 1;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo; 

void printLCD(int col, int row , char *str) {
    for(int i=0 ; i < strlen(str) ; i++){
      lcd.setCursor(col+i , row);
      lcd.print(str[i]);
    }
}

void setup() {
  // HW I/O define   
  pinMode(LIGHTPIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(RBG_R,OUTPUT);
  pinMode(RBG_G,OUTPUT);
	pinMode(RBG_B,OUTPUT);
  
  dht.begin();
  analogWrite(LIGHTPIN, 255);
  
  lcd.init();
  lcd.backlight();
  printLCD(0, 0, "Smart Farm ");
  printLCD(0, 1, "Are You Ready? "); 

  Serial.begin(9600); // Serial Monitor for Debug  
  Serial1.begin(9600);  //Bluetooth Module
  Serial.println("START");
  digitalWrite(FAN_PIN, 1);
 }



void loop() { // put your main code here, to run repeatedly:
	
	timeout += 1;
	if(timeout % 10 == 0) {

	cdcValue = analogRead(A0); // Analog adc Value 
	//Serial.print(cdcValue); Serial.print(",");
	  
	waterValue = analogRead(A1);
  waterValue = 100 - waterValue/10; //% Display
	// Serial.print(waterValue); Serial.print(",");
   
	humidity = dht.readHumidity();
	temperature = dht.readTemperature();

	lcd.clear();
  memset(sData, 0x00, 64);
  sprintf(sData, "tem %02dC Pho %04d ", (int)temperature,cdcValue) ;
  printLCD(0, 0, sData);
  memset(sData, 0x00, 64);
  sprintf(sData, "hum %02d%% soi %03d%%", (int)humidity, waterValue);
  printLCD(0, 1, sData);

 sprintf(sData,"{ \"temp\":%02d,\"humidity\":%02d,\"cdc\":%-04d,\"water\":%-04d,\"co2\":%-04d }", 
      (int)temperature, (int)humidity,cdcValue, waterValue, get_co2_ppm); 
      //%-04d 변수 4자리 표현 (-) 0표시를 제거하는 명령, (int)temperature 형변환 foat=>int      
	
	Serial1.println(sData); // Bluetooth Tx 
  }

 // Bluetooth Received Data 
	  while(0 < Serial1.available()) {
	  	char ch = Serial1.read();
  		rData[rPos] = ch;
  		rPos += 1;
  		Serial.print(ch); // 문자열을 출력 
  
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
  				if(rData[4] == '0') angle = 10;
  				else angle = 80;
  				servo.attach(SERVOPIN);
  				servo.write(angle); 
  				delay(500);
  				servo.detach();
  				#if DEBUG
  				Serial.print("server_f_MOTOR=");
  				Serial.println(angle);
  				#endif
  				
  			}
  			
  			if(memcmp(rData, "C_F-", 4) == 0)
  			{
  			
  				if(rData[4] == '0') 
  				digitalWrite(FAN_PIN, 0);
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
  			
  				if(rData[4] == '0') 
  				digitalWrite(WATER_PUMP_PIN, 0);
  				
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
  			memset(rData, 0x00, 32);
    			break;
  		}
   		delay(10);
	  }

  // water_pump timer 5 Minutes
  if (water_State){
    water_Time +=1; 
     if ((water_Time) > 2500){
     digitalWrite(WATER_PUMP_PIN, 0);
     water_State = false; 
     water_Time =0;
    }
  }
/*
  local_time = millis(); 
  Serial.print("local_time = ");
  Serial.println(local_time);
  */
  delay(100);     
}
