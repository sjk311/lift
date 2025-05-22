// === Z축 제어 핀 ===
const int Y_LEFT_PUL = 13;
const int Y_LEFT_DIR = 12;
const int Y_RIGHT_PUL = 11;
const int Y_RIGHT_DIR = 10;

// === 엘리베이터 제어 핀 ===
const int ELEV_PUL = 9;
const int ELEV_DIR = 8;
const int sensorPins[4] = {4, 5, 6, 7}; // 1~4층 센서

// === 센서 핀 ===
const int Y_LEFT_SENSOR_BACK = 22;
const int Y_LEFT_SENSOR_FRONT = 24;
const int Y_RIGHT_SENSOR_BACK = 26;
const int Y_RIGHT_SENSOR_FRONT = 28;

// === 설정값 ===
const int frequency = 2000;
const int elevFrequency = 500;

int currentFloor = 0;
bool isMovingElevator = false;
bool motorRunning = false;
char currentCommand = '0';

void setup() {
  // Z축
  pinMode(Y_LEFT_PUL, OUTPUT);
  pinMode(Y_LEFT_DIR, OUTPUT);
  pinMode(Y_RIGHT_PUL, OUTPUT);
  pinMode(Y_RIGHT_DIR, OUTPUT);
  
  // 엘리베이터
  pinMode(ELEV_PUL, OUTPUT);
  pinMode(ELEV_DIR, OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(sensorPins[i], INPUT);  // 그대로 유지

  // Y축 센서만 INPUT_PULLUP으로 변경
  pinMode(Y_LEFT_SENSOR_BACK, INPUT_PULLUP);
  pinMode(Y_LEFT_SENSOR_FRONT, INPUT_PULLUP);
  pinMode(Y_RIGHT_SENSOR_BACK, INPUT_PULLUP);
  pinMode(Y_RIGHT_SENSOR_FRONT, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("시스템 시작. 명령어 입력:");
}

void loop() {
  updateCurrentFloor();

  if (Serial.available() > 0) {
    char command = Serial.read();
    currentCommand = command;
    motorRunning = (command != '0');

    if (command >= '1' && command <= '4') {
      moveToFloor(command - '0');
    } else if (command == '0') {
      sFRONTAllMotors();
    }
  }

  // Z축 모터 동작 처리
  if (motorRunning) {
    if (currentCommand == '5') {
      Serial.println("Z축 왼쪽 하강 중...");
      digitalWrite(Y_LEFT_DIR, HIGH);
      while (digitalRead(Y_LEFT_SENSOR_BACK) == HIGH) generateY_LEFT_PULse();
      Serial.println("BACK");
      motorRunning = false;
    } 
    else if (currentCommand == '6') {
      Serial.println("Z축 왼쪽 상승 중...");
      digitalWrite(Y_LEFT_DIR, LOW);
      while (digitalRead(Y_LEFT_SENSOR_FRONT) == HIGH) generateY_LEFT_PULse();
      Serial.println("FRONT");
      motorRunning = false;
    } 
    else if (currentCommand == '7') {
      Serial.println("Z축 오른쪽 하강 중...");
      digitalWrite(Y_RIGHT_DIR, HIGH);
      while (digitalRead(Y_RIGHT_SENSOR_BACK) == HIGH) generateY_RIGHT_PULse();
      Serial.println("BACK");
      motorRunning = false;
    } 
    else if (currentCommand == '8') {
      Serial.println("Z축 오른쪽 상승 중...");
      digitalWrite(Y_RIGHT_DIR, LOW);
      while (digitalRead(Y_RIGHT_SENSOR_FRONT) == HIGH) generateY_RIGHT_PULse();
      Serial.println("FRONT");
      motorRunning = false;
    }
  }
}

// === 펄스 생성 함수들 ===
void generateY_LEFT_PULse() {
  int period_us = 1000000 / frequency;
  digitalWrite(Y_LEFT_PUL, HIGH); delayMicroseconds(period_us / 2);
  digitalWrite(Y_LEFT_PUL, LOW); delayMicroseconds(period_us / 2);
}

void generateY_RIGHT_PULse() {
  int period_us = 1000000 / frequency;
  digitalWrite(Y_RIGHT_PUL, HIGH); delayMicroseconds(period_us / 2);
  digitalWrite(Y_RIGHT_PUL, LOW); delayMicroseconds(period_us / 2);
}

void generateElevPulse() {
  int period_us = 1000000 / elevFrequency;
  digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(period_us / 2);
  digitalWrite(ELEV_PUL, LOW); delayMicroseconds(period_us / 2);
}

// === 현재 층 업데이트 ===
void updateCurrentFloor() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(sensorPins[i]) == LOW) {
      if (currentFloor != i + 1) {
        currentFloor = i + 1;
        Serial.print("현재 층: ");
        Serial.println(currentFloor);
      }
      break;
    }
  }
}

// === 엘리베이터 이동 ===
void moveToFloor(int targetFloor) {
  if (targetFloor == currentFloor) {
    Serial.println("이미 해당 층입니다.");
    return;
  }

  isMovingElevator = true;

  if (targetFloor > currentFloor) {
    Serial.println("상층으로 이동 중...");
    digitalWrite(ELEV_DIR, HIGH);
  } else {
    Serial.println("하층으로 이동 중...");
    digitalWrite(ELEV_DIR, LOW);
  }

  while (isMovingElevator) {
    generateElevPulse();
    updateCurrentFloor();
    if (currentFloor == targetFloor) {
      Serial.print("도착 층: ");
      Serial.println(currentFloor);
      isMovingElevator = false;
    }
  }
}

// === 전체 모터 정지 ===
void sFRONTAllMotors() {
  digitalWrite(Y_LEFT_PUL, LOW);
  digitalWrite(Y_RIGHT_PUL, LOW);
  digitalWrite(ELEV_PUL, LOW);
  Serial.println("모터 정지됨.");
}
