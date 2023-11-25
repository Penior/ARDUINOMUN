#include <Servo.h>
#include <DHT.h>
#define DHTPIN 12
#define DHTTYPE DHT11
#define SERVOPIN 9
#define LIGHTPIN 4
#define FAN_PIN 32
#define WATER_PUMP_PIN 31
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define USE_NETWORK 1
#define USE_BLUETOOTH 1
#define DEBUG 1

float temperature, humidity;
int angle = 0;
int get_co2_ppm = 0; 
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
//========================================================
#include "WiFiEsp.h"
char ssid[] = "KT_GiGA_5149";   // your network SSID (name)
char pass[] = "cf2agd1388";  // your network password
//char ssid[] = "SMARTFARM";   // your network SSID (name)
//char pass[] = "blueinno";  // your network password
int status = WL_IDLE_STATUS;  // the Wifi radio's status
WiFiEspServer server_f(400);
//========================================================

DHT dht(DHTPIN, DHTTYPE);
Servo servo; 
LiquidCrystal_I2C lcd(0x27, 16, 2);

void printLCD(int col, int row , char *str) {
    for(int i=0 ; i < strlen(str) ; i++){
      lcd.setCursor(col+i , row);
      lcd.print(str[i]);
    }}

void printWifiStatus(){
#if 1 // #if DEBUG
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
#endif

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  delay(10);
#if 1 // #if DEBUG
  Serial.print("IP Address: ");
  Serial.println(ip);
#endif
	char ipno2[26] ;
	sprintf(ipno2, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  	printLCD(0, 1, ipno2);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  
#if 1 // #if DEBUG
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
#endif
}

void setup() {
  pinMode(LIGHTPIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  dht.begin();
  analogWrite(LIGHTPIN, 255);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  lcd.init();
  lcd.backlight();
  printLCD(0, 0, "MMB_SmartFarm");
  printLCD(0, 1, "NETWORKING...");  

#if USE_NETWORK
  // initialize serial for ESP module
  Serial2.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial2);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
  	#if DEBUG
    Serial.println("WiFi shield not present");
	#endif
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
  	#if DEBUG
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
	#endif
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
	#if DEBUG
  Serial.println("You're connected to the network");
	#endif
  printWifiStatus(); // display IP address on LCD
  delay(2000);
	
  server_f.begin(); 
#endif
  
  #if DEBUG
  Serial.println("START");
  #endif
}

void loop() { 
  // put your main code here, to run repeatedly:
		timeout += 1;
	  if(timeout % 10 == 0) {

	  cdcValue = analogRead(0);
	  cdcValue /= 10;
	  //Serial.print(cdcValue); Serial.print(",");
	  
	  waterValue = analogRead(1);
	  waterValue /= 10;
	  //Serial.print(waterValue); Serial.print(",");

	  humidity = dht.readHumidity();
	  temperature = dht.readTemperature();

	  lcd.clear();
	  displayToggle = !displayToggle;
	  if(displayToggle == 1) {
		  memset(sData, 0x00, 64);
		  sprintf(sData, "temp %02dC humi %02d%%", (int)temperature,
		  (int)humidity);
		  printLCD(0, 0, sData);
		  memset(sData, 0x00, 64);
		  sprintf(sData, "cdc%-04d soil%-04d", cdcValue, waterValue);
		  printLCD(0, 1, sData);
	  }
	  else {
		  memset(sData, 0x00, 64);
		  sprintf(sData, "temp %02dC humi %02d%%", (int)temperature, 
		  (int)humidity);
		  printLCD(0, 0, sData);
		  memset(sData, 0x00, 64);
		  sprintf(sData, "co2 %d ppm", get_co2_ppm);
		  printLCD(0, 1, sData);
	  }

    sprintf(sData, "{ \"temp\":%02d,\"humidity\":%02d,\"cdc\":%-04d,\"water\":%-04d,\"co2\":%-04d }", 
      (int)temperature, (int)humidity,
      cdcValue, waterValue, get_co2_ppm);
     Serial.println(sData);

	
    Serial1.println(sData);
  }

	  while(0 < Serial1.available()) {
	  	
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
  				else { 
         digitalWrite(WATER_PUMP_PIN, 1);
         water_State = true;}
         
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
  
	#if USE_NETWORK
  	WiFiEspClient c = server_f.available();
	if(c) 
	{
		#if DEBUG
	  Serial.println("N#RECV: ");
		#endif
	  boolean bDataRead = false;

		nPos = 0;
	  
	  while (0 < c.available()) {
	    	char ch = c.read();
			#if DEBUG
	    	Serial.write(ch);
			#endif
			bDataRead = true;
			nData[nPos] = ch; nPos += 1;
	  }

	  Serial.print("nData=");
	  Serial.println(nData);
	  if(bDataRead == true) {
	  	#if DEBUG
	  	Serial.println();
		#endif
  		// sprintf(sData, "timeout %d", timeout);
  		c.print(sData);
    
  		if(5 <= nPos)
  		{	
  			#if DEBUG
  			Serial.println(nData);
  			#endif
			
  			if(memcmp(nData, "C_S-", 4) == 0)
  			{
  			
  				if(nData[4] == '0') angle = 10;
  				else angle = 80;
  				servo.attach(SERVOPIN);
  				servo.write(angle); 
  				delay(500);
  				servo.detach();
  				#if DEBUG
  				Serial.print("N#server_f_MOTOR=");
  				Serial.println(angle);
  				#endif
  			}
  			
  			if(memcmp(nData, "C_F-", 4) == 0)
  			{
  			
  				if(nData[4] == '0') digitalWrite(FAN_PIN, 0);
  				else digitalWrite(FAN_PIN, 1);
  				#if DEBUG
  				Serial.print("N#FAN=");
  				Serial.println(nData[4]);
  				#endif
  				
  			}
  
  			if(memcmp(nData, "C_L-", 4) == 0)
  			{
				  int light = atoi(nData+4);
				  analogWrite(LIGHTPIN, (int)(25 * light));

				  #if 1 // #if DEBUG
				  Serial.print("N#LIGHT=");
				  Serial.println(light);
				  #endif
  				
  			} 
  
  			if(memcmp(nData, "C_W-", 4) == 0)
  			{
  			
  				if(nData[4] == '0') digitalWrite(WATER_PUMP_PIN, 0);
  				else { 
  				digitalWrite(WATER_PUMP_PIN, 1);
         water_State = true;}
         
  				#if DEBUG
  				Serial.print("N#WATER=");
  				Serial.println(nData[4]);
  				#endif
  				
  			}
  			
  
  			nPos = 0;
  			memset(nData, 0x00, 32);
  		}
	  }
	  
	  delay(10);

	  // close the connection:
	  // c.stop();
	  // Serial.println("Client disconnected");
	}
	#endif

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
