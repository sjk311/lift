int PUL = 10;          // 펄스 신호 핀
int DIR = 13;          // 방향 제어 핀
int microStep = 8;     // 마이크로스텝 설정
int frequency = 700;   // 펄스 주파수 (Hz)
int duration = 1000;   // 동작 지속 시간 (ms)
bool isStopped = true; // 모터 정지 상태 확인

void setup() {
  pinMode(PUL, OUTPUT); // 펄스 핀을 출력으로 설정
  pinMode(DIR, OUTPUT); // 방향 핀을 출력으로 설정
  Serial.begin(9600);   // 시리얼 통신 시작 (9600bps)
  Serial.println("Ready: Enter 1 for Forward, 2 for Reverse, 0 to Stop");
}

void loop() {
  if (Serial.available() > 0) { // 입력이 있을 경우
    char input = Serial.read(); // 입력값 읽기

    if (input == '1') { // 입력값이 '1'일 경우 (전진)
      Serial.println("Forward: 2 seconds");
      digitalWrite(DIR, HIGH); // DIR 핀 HIGH → 전진
      isStopped = false;       // 정지 상태 해제
      generatePulse(duration); // 2초 동안 펄스 생성
    } 
    else if (input == '2') { // 입력값이 '2'일 경우 (후진)
      Serial.println("Reverse: 2 seconds");
      digitalWrite(DIR, LOW); // DIR 핀 LOW → 후진
      isStopped = false;      // 정지 상태 해제
      generatePulse(duration); // 2초 동안 펄스 생성
    } 
    else if (input == '0') { // 입력값이 '0'일 경우 (정지)
      Serial.println("Stop");
      isStopped = true; // 정지 상태 설정
      stopMotor();      // 모터 정지 함수 호출
    } 
    else { // 유효하지 않은 입력값
      Serial.println("Invalid input. Enter 1 (Forward), 2 (Reverse), or 0 (Stop).");
    }
  }
}

// 펄스 생성 함수
void generatePulse(int duration_ms) {
  unsigned long startTime = millis(); // 시작 시간 저장
  int period_us = 1000000 / frequency; // 펄스 주기 계산 (마이크로초)
  while (millis() - startTime < duration_ms && !isStopped) { // 지정된 시간 동안 반복, 정지 상태가 아닐 경우
    digitalWrite(PUL, HIGH);
    delayMicroseconds(period_us / 2); // HIGH 유지 시간
    digitalWrite(PUL, LOW);
    delayMicroseconds(period_us / 2); // LOW 유지 시간
  }
}

// 모터 정지 함수
void stopMotor() {
  digitalWrite(PUL, LOW); // 펄스 신호를 LOW로 유지하여 모터 정지
}
