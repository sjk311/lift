int PUL = 10;          // 펄스 신호 핀
int DIR = 13;          // 방향 제어 핀
int sensorPins[4] = {4,5,6,7}; // 각 층의 센서 핀 (NC 접점 연결)
int microStep = 8;     // 마이크로스텝 설정
int frequency = 700;   // 펄스 주파수 (Hz)
int currentFloor = 0;  // 현재 층 (0: 초기 상태)
bool isMoving = false; // 이동 중 상태 확인

void setup() {
  pinMode(PUL, OUTPUT);    // 펄스 핀을 출력으로 설정
  pinMode(DIR, OUTPUT);    // 방향 핀을 출력으로 설정

  // 모든 센서 핀을 입력으로 설정
  for (int i = 0; i < 4; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  Serial.begin(9600);      // 시리얼 통신 시작 (9600bps)
  Serial.println("System Ready. Enter a floor (1 to 4).");
}

void loop() {
  // 현재 층 감지
  updateCurrentFloor();

  // 시리얼 입력 처리
  if (Serial.available() > 0) {
    char input = Serial.read(); // 입력값 읽기
    int targetFloor = input - '0'; // 입력값을 숫자로 변환

    if (targetFloor >= 1 && targetFloor <= 4) { // 유효한 층 입력
      Serial.print("Moving to floor: ");
      Serial.println(targetFloor);

      moveToFloor(targetFloor); // 목표 층으로 이동
    } 


    else {
      Serial.println("Invalid floor. Enter a number between 1 and 4.");
    }
  }
}

// 현재 층 업데이트 함수
void updateCurrentFloor() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(sensorPins[i]) == LOW) { // 센서 감지
      if (currentFloor != i + 1) { // 층이 변경된 경우
        currentFloor = i + 1; // 현재 층 업데이트
        Serial.print("Current Floor: ");
        Serial.println(currentFloor);
      }
      break; // 한 층만 감지
    }
  }
}

// 목표 층으로 이동하는 함수
void moveToFloor(int targetFloor) {
  if (targetFloor == currentFloor) {
    Serial.println("Already at the target floor.");
    return; // 이동 불필요
  }

  isMoving = true;

  // 이동 방향 설정
  if (targetFloor > currentFloor) {
    Serial.println("Moving up...");
    digitalWrite(DIR, HIGH); // 위로 이동
  } else {
    Serial.println("Moving down...");
    digitalWrite(DIR, LOW); // 아래로 이동
  }

  // 목표 층 도달까지 이동
  while (isMoving) {
    generatePulse(); // 펄스 생성

    // 현재 층을 업데이트하여 목표 층 도달 확인
    updateCurrentFloor();
    if (currentFloor == targetFloor) {
      Serial.print("Arrived at floor: ");
      Serial.println(currentFloor);
      isMoving = false; // 이동 중단
    }
  }
}

// 펄스 생성 함수 (연속 동작)
void generatePulse() {
  int period_us = 1000000 / frequency; // 펄스 주기 계산 (마이크로초)
  digitalWrite(PUL, HIGH);
  delayMicroseconds(period_us / 2);    // HIGH 유지 시간
  digitalWrite(PUL, LOW);
  delayMicroseconds(period_us / 2);    // LOW 유지 시간
}
