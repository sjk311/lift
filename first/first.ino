int PUL = 10;          // 펄스 신호 핀
int DIR = 13;          // 방향 제어 핀
int microStep = 8;     // 마이크로스텝 설정
int frequency = 50000;   // 펄스 주파수 (Hz)
int duration = 3000;   // 전진/후진 지속 시간 (ms)

void setup() {
  pinMode(PUL, OUTPUT); // 펄스 핀을 출력으로 설정
  pinMode(DIR, OUTPUT); // 방향 핀을 출력으로 설정
}

void loop() {
  // 1. 전진 방향
  digitalWrite(DIR, HIGH); // DIR 핀 HIGH → 전진
  generatePulse(duration); // 3초 동안 펄스 생성

  // 2. 후진 방향
  //digitalWrite(DIR, LOW);  // DIR 핀 LOW → 후진
  //generatePulse(duration); // 3초 동안 펄스 생성
}

// 펄스 생성 함수
void generatePulse(int duration_ms) {
  unsigned long startTime = millis(); // 시작 시간 저장
  int period_us = 1000000 / frequency; // 펄스 주기 계산 (마이크로초)
  while (millis() - startTime < duration_ms) { // 지정된 시간 동안 반복
    digitalWrite(PUL, HIGH);
    delayMicroseconds(period_us / 2); // HIGH 유지 시간
    digitalWrite(PUL, LOW);
    delayMicroseconds(period_us / 2); // LOW 유지 시간
  }
}
