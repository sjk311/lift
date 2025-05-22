int PUL = 10;          // 펄스 신호 핀
int DIR = 13;          // 방향 제어 핀
int sensorPins[4] = {7, 6, 5, 4}; // 4개의 센서 핀 (NC 접점 연결)
int microStep = 8;     // 마이크로스텝 설정
int frequency = 2500;   // 펄스 주파수 (Hz)
bool isStopped = true; // 모터 정지 상태 확인
bool isContinuous = false; // 연속 동작 여부

void setup() {
  pinMode(PUL, OUTPUT);    // 펄스 핀을 출력으로 설정
  pinMode(DIR, OUTPUT);    // 방향 핀을 출력으로 설정

  Serial.begin(9600);      // 시리얼 통신 시작 (9600bps)
  Serial.println("Ready: Enter 1 for Forward, 2 for Reverse, 3 for Continuous Forward, 4 for Continuous Reverse, 0 to Stop");

  // 모든 센서 핀을 입력으로 설정
  for (int i = 0; i < 4; i++) {
    pinMode(sensorPins[i], INPUT);
  }

}

void loop() {
  if (Serial.available() > 0) { // 입력이 있을 경우
    char input = Serial.read(); // 입력값 읽기

    if (input == '1') { // 1: 전진 (한 번만 동작)
      Serial.println("Forward: 2 seconds");
      digitalWrite(DIR, HIGH); // DIR 핀 HIGH → 전진
      isStopped = false;       // 정지 상태 해제
      generatePulse(2000);     // 2초 동안 펄스 생성
    } 
    else if (input == '2') { // 2: 후진 (한 번만 동작)
      Serial.println("Reverse: 2 seconds");
      digitalWrite(DIR, LOW);  // DIR 핀 LOW → 후진
      isStopped = false;       // 정지 상태 해제
      generatePulse(2000);     // 2초 동안 펄스 생성
    } 
    else if (input == '3') { // 3: 계속 전진
      Serial.println("Continuous Forward");
      digitalWrite(DIR, HIGH); // DIR 핀 HIGH → 전진
      isStopped = false;       // 정지 상태 해제
      isContinuous = true;     // 연속 동작 활성화
    } 
    else if (input == '4') { // 4: 계속 후진
      Serial.println("Continuous Reverse");
      digitalWrite(DIR, LOW);  // DIR 핀 LOW → 후진
      isStopped = false;       // 정지 상태 해제
      isContinuous = true;     // 연속 동작 활성화
    } 
    else if (input == '0') { // 0: 정지
      Serial.println("Stop");
      isStopped = true;       // 정지 상태 설정
      isContinuous = false;   // 연속 동작 비활성화
      stopMotor();            // 모터 정지 함수 호출
    } 
    else { // 유효하지 않은 입력값
      Serial.println("Invalid input. Enter 1, 2, 3, 4, or 0.");
    }
  }

  // 연속 동작 처리
  if (isContinuous && !isStopped) {
    if (checkSensors()) { // 접점 감지 여부 확인
      isStopped = true;      // 정지 상태 설정
      isContinuous = false;  // 연속 동작 비활성화
      stopMotor();           // 모터 정지
    } else {
      generateContinuousPulse(); // 연속 펄스 생성
    }
  }
}

// 접점 감지 여부 확인 함수
bool checkSensors() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(sensorPins[i]) == LOW) { // 접점 감지 (센서 출력 LOW)
      Serial.print("Sensor on pin "); Serial.print(sensorPins[i]); Serial.println(" detected!");
      return true; // 접점 감지 시 true 반환
    }
  }
  return false; // 감지되지 않으면 false 반환
}

// 연속 펄스 생성 함수
void generateContinuousPulse() {
  int period_us = 1000000 / frequency; // 펄스 주기 계산 (마이크로초)
  digitalWrite(PUL, HIGH);
  delayMicroseconds(period_us / 2);    // HIGH 유지 시간
  digitalWrite(PUL, LOW);
  delayMicroseconds(period_us / 2);    // LOW 유지 시간
}

// 펄스 생성 함수 (지정된 시간 동안)
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
