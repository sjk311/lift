#include <Adafruit_NeoPixel.h>

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

// === 네오픽셀 설정 ===
#define LEFT_LED_PIN   48
#define RIGHT_LED_PIN  47
#define NUMPIXELS      12

Adafruit_NeoPixel leftStrip(NUMPIXELS, LEFT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightStrip(NUMPIXELS, RIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);

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

  leftStrip.begin();
  rightStrip.begin();
  leftStrip.show();   // 초기 OFF
  rightStrip.show();
}

void loop() {
  updateCurrentFloor();

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "r") {
      setBothPanels(255, 0, 0);
      Serial.println("LED: 빨강");
    } else if (input == "g") {
      setBothPanels(0, 255, 0);
      Serial.println("LED: 초록");
    } else if (input == "b") {
      setBothPanels(0, 0, 255);
      Serial.println("LED: 파랑");
    } else if (input == "y") {
      setBothPanels(255, 255, 0);
      Serial.println("LED: 노랑");
    } else if (input == "off") {
      turnOffPanels();
      Serial.println("LED: OFF");
    } else {
      int command = input.toInt();
      currentCommand = command;
      motorRunning = (command != 0);

      switch (command) {
        case 0:
          sFRONTAllMotors();
          break;
        case 1: case 2: case 3: case 4:
          moveToFloor(command);
          break;
        case 10:
          digitalWrite(L_solenoid, HIGH); Serial.println("L_solenoid ON"); break;
        case 11:
          digitalWrite(L_solenoid, LOW); Serial.println("L_solenoid OFF"); break;
        case 12:
          digitalWrite(R_solenoid, HIGH); Serial.println("R_solenoid ON"); break;
        case 13:
          digitalWrite(R_solenoid, LOW); Serial.println("R_solenoid OFF"); break;
      }
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

// === 네오픽셀 함수 ===
void setBothPanels(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    leftStrip.setPixelColor(i, leftStrip.Color(r, g, b));
    rightStrip.setPixelColor(i, rightStrip.Color(r, g, b));
  }
  leftStrip.show();
  rightStrip.show();
}

void turnOffPanels() {
  setBothPanels(0, 0, 0);
}

// === 펄스 생성 ===
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

// === 층 정보 갱신 ===
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

// === 엘리베이터 층 이동 ===
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
