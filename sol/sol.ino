const int RELAY1_PIN = 5;  // 1번 전자석 제어 릴레이
const int RELAY2_PIN = 6;  // 2번 전자석 제어 릴레이

void setup() {
  Serial.begin(9600);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // 초기 상태: 릴레이 OFF (HIGH)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();

    switch (input) {
      case '0':
        digitalWrite(RELAY1_PIN, HIGH);  // 둘 다 끄기
        digitalWrite(RELAY2_PIN, HIGH);
        break;

      case '1':
        digitalWrite(RELAY1_PIN, LOW);   // 1번 켜기
        break;

      case '2':
        digitalWrite(RELAY1_PIN, HIGH);  // 1번 끄기
        break;

      case '3':
        digitalWrite(RELAY2_PIN, LOW);   // 2번 켜기
        break;

      case '4':
        digitalWrite(RELAY2_PIN, HIGH);  // 2번 끄기
        break;

      default:
        // 아무것도 안 함
        break;
    }
  }
}
