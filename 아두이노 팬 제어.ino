#define FAN_PIN 32  // 팬이 연결된 PWM 핀 번호

void setup() {
  Serial.begin(9600);
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case '0':
        fanOff();
        break;
      case '1':
        fanON();
        break;
    }
  }
}

void fanOff() {
  analogWrite(FAN_PIN, 0);
}

void fanON() {
  analogWrite(FAN_PIN, 255);  // 예시로 PWM 신호의 100% 사용
}
