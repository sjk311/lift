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

// === 전자석 핀 ===
const int L_solenoid = 32;
const int R_solenoid = 34;

// === 설정값 ===
const int frequency = 4000;
const int elevFrequency = 2000;

int currentFloor = 0;
bool isMovingElevator = false;
bool motorRunning = false;
int currentCommand = 0;

void setup() {
  pinMode(Y_LEFT_PUL, OUTPUT);
  pinMode(Y_LEFT_DIR, OUTPUT);
  pinMode(Y_RIGHT_PUL, OUTPUT);
  pinMode(Y_RIGHT_DIR, OUTPUT);

  pinMode(ELEV_PUL, OUTPUT);
  pinMode(ELEV_DIR, OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(sensorPins[i], INPUT);

  pinMode(Y_LEFT_SENSOR_BACK, INPUT_PULLUP);
  pinMode(Y_LEFT_SENSOR_FRONT, INPUT_PULLUP);
  pinMode(Y_RIGHT_SENSOR_BACK, INPUT_PULLUP);
  pinMode(Y_RIGHT_SENSOR_FRONT, INPUT_PULLUP);

  pinMode(L_solenoid, OUTPUT);
  pinMode(R_solenoid, OUTPUT);

  Serial.begin(9600);
  Serial.println("시스템 시작. 명령어 입력:");
}

void loop() {
  updateCurrentFloor();

  if (Serial.available() > 0) {
    int command = Serial.parseInt();
    currentCommand = command;
    motorRunning = (command != 0);

    switch (command) {
      case 0:
        sFRONTAllMotors();
        break;
      case 1: case 2: case 3: case 4:
        moveToFloor(command);
        break;
      case 10:    //왼쪽 전자석  off
        digitalWrite(L_solenoid, HIGH);
        Serial.println("L_solenoid OFF");
        break;
      case 11:    //왼쪽 전자석  off
        digitalWrite(L_solenoid, LOW);
        Serial.println("L_solenoid ON");
        break;
      case 12:    //오른쪽 전자석  on
        digitalWrite(R_solenoid, HIGH);
        Serial.println("R_solenoid OFF");
        break;
      case 13:    //오른쪽 전자석  Off
        digitalWrite(R_solenoid, LOW);
        Serial.println("R_solenoid ON");
        break;
    }
  }

  if (motorRunning) {
    switch (currentCommand) {
      case 5:
        Serial.println("Z축 왼쪽 하강 중...");
        digitalWrite(Y_LEFT_DIR, HIGH);
        while (digitalRead(Y_LEFT_SENSOR_BACK) == HIGH) generateY_LEFT_PULse();
        Serial.println("BACK");
        motorRunning = false;
        break;
      case 6:
        Serial.println("Z축 왼쪽 상승 중...");
        digitalWrite(Y_LEFT_DIR, LOW);
        while (digitalRead(Y_LEFT_SENSOR_FRONT) == HIGH) generateY_LEFT_PULse();
        Serial.println("FRONT");
        motorRunning = false;
        break;
      case 7:
        Serial.println("Z축 오른쪽 하강 중...");
        digitalWrite(Y_RIGHT_DIR, HIGH);
        while (digitalRead(Y_RIGHT_SENSOR_BACK) == HIGH) generateY_RIGHT_PULse();
        Serial.println("BACK");
        motorRunning = false;
        break;
      case 8:
        Serial.println("Z축 오른쪽 상승 중...");
        digitalWrite(Y_RIGHT_DIR, LOW);
        while (digitalRead(Y_RIGHT_SENSOR_FRONT) == HIGH) generateY_RIGHT_PULse();
        Serial.println("FRONT");
        motorRunning = false;
        break;
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
