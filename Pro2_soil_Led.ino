/* SmartFarm MMB BD Pro2 Soil_Led
   www.blueinno.co.kr 
   2020. 11. 10 
 */
float waterAdc; 
int waterValue;

int red = 23;
int green = 35; 
void setup() {
  Serial.begin(9600);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
 }

void loop(){
  int soil = analogRead(A1);
  Serial.println(soil);
  if (soil > 600) {
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    delay(200);  
    digitalWrite(red, LOW);
  }
  else {
    digitalWrite(green, HIGH);
  }
  delay(500);
}
