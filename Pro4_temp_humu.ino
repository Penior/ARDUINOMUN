#include <DHT.h>
DHT dht(12, DHT11);

int request_dht11()
{
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  Serial.print("humidity=");
  Serial.println(h);
  Serial.print("temperature = ");
  Serial.println(t);
}
void setup(){
   Serial.begin(9600);
   dht.begin();
}

void loop(){
 request_dht11();
 delay(1000);
         }

   
