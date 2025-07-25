// === 라이브러리 ===
#include <Adafruit_NeoPixel.h>

// === 핀 설정 ===
const int floorSensorPins[4] = {4, 5, 6, 7};           // 1~4층 센서
const int ySensorPins[4] = {22, 24, 26, 28};           // Y축 센서
const int Y_LEFT_PUL = 13, Y_LEFT_DIR = 12;
const int Y_RIGHT_PUL = 11, Y_RIGHT_DIR = 10;
const int ELEV_PUL = 9, ELEV_DIR = 8;
const int L_solenoid = 32, R_solenoid = 34;

// === 네오픽셀 설정 ===
#define LEFT_LED_PIN   48
#define RIGHT_LED_PIN  47
#define NUMPIXELS      12
Adafruit_NeoPixel leftStrip(NUMPIXELS, LEFT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightStrip(NUMPIXELS, RIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);

// === 레이블 ===
const char* floorLabels[4] = {"1층", "2층", "3층", "4층"};
const char* yLabels[4] = {"왼쪽 BACK", "왼쪽 FRONT", "오른쪽 BACK", "오른쪽 FRONT"};

// === 필터 관련 ===
float filteredFloor[4] = {1, 1, 1, 1};
float filteredY[4] = {1, 1, 1, 1};
int prevRawFloor[4] = {HIGH, HIGH, HIGH, HIGH};
int prevFilteredFloor[4] = {HIGH, HIGH, HIGH, HIGH};
int prevRawY[4] = {HIGH, HIGH, HIGH, HIGH};
int prevFilteredY[4] = {HIGH, HIGH, HIGH, HIGH};
const float alpha = 0.9;
const float threshold = 0.3;

// === 기타 설정 ===
const int frequency = 7000;
const int elevFrequency = 2000;
const int pulse_delay = 1000;

bool motorRunning = false;
int currentCommand = 0;
int currentFloor = 0;
bool isMovingElevator = false;

// === setup ===
void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    pinMode(floorSensorPins[i], INPUT_PULLUP);
    pinMode(ySensorPins[i], INPUT_PULLUP);
  }

  pinMode(Y_LEFT_PUL, OUTPUT);  pinMode(Y_LEFT_DIR, OUTPUT);
  pinMode(Y_RIGHT_PUL, OUTPUT); pinMode(Y_RIGHT_DIR, OUTPUT);
  pinMode(ELEV_PUL, OUTPUT);    pinMode(ELEV_DIR, OUTPUT);
  pinMode(L_solenoid, OUTPUT);  pinMode(R_solenoid, OUTPUT);

  leftStrip.begin(); rightStrip.begin();
  leftStrip.show(); rightStrip.show();

  setBothPanels(0, 0, 0);   // 초기 led off
  digitalWrite(R_solenoid, HIGH);   // 전자석 끄기
  digitalWrite(L_solenoid, HIGH);   // 전자석 끄기

  Serial.println("=== 시스템 시작됨 ===");
}

// === loop ===
void loop() {
  updateCurrentFloor();
  updateAndPrintYSensors();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    motorRunning = (input.toInt() != 0);

    handleSerialInput(input);
  }

  if (motorRunning) checkAndRunMotor();
  delay(10);
}

// === Serial 입력 처리 ===
void handleSerialInput(String input) {
  if (input == "r") setBothPanels(255, 0, 0);
  else if (input == "g") setBothPanels(0, 255, 0);
  else if (input == "b") setBothPanels(0, 0, 255);
  else if (input == "y") setBothPanels(255, 255, 0);
  else if (input == "w") setBothPanels(255, 255, 255);
  else if (input == "off") turnOffPanels();
  else if (input.startsWith("u ") || input.startsWith("d ")) handleManualElevator(input);
  else handleCommand(input.toInt());
}

void handleManualElevator(String input) {
  int spaceIndex = input.indexOf(' ');    // 슬라이싱 기준 (띄어쓰기)
  String direction = input.substring(0, spaceIndex);      // 방향 저장
  int cntLimit = input.substring(spaceIndex + 1).toInt();   // 펄스 수 저장

  if (cntLimit <= 0) {
    Serial.println("잘못된 펄스 수입니다.");
    return;
  }

  digitalWrite(ELEV_DIR, (direction == "u") ? HIGH : LOW);
  Serial.print("ELEV "); Serial.print(direction == "u" ? "UP " : "DOWN ");
  Serial.print(cntLimit); Serial.println(" 펄스 이동 중...");

  for (int i = 0; i < cntLimit; i++) {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay);
  }

  Serial.println("ELEV 이동 완료");
}

void handleCommand(int cmd) {
  currentCommand = cmd;
  switch (cmd) {
    case 0: stopAllMotors(); break;
    case 1: case 2: case 3: case 4: moveToFloor(cmd); break;

    case 10: digitalWrite(L_solenoid, HIGH); break;     // 왼쪽 전자석 off
    case 11: digitalWrite(L_solenoid, LOW); break;     // 왼쪽 전자석 on
    case 12: digitalWrite(R_solenoid, HIGH); break;    // 오른쪽 전자석 off
    case 13: digitalWrite(R_solenoid, LOW); break;     // 오른쪽 전자석 on

    case 30: PickupRoutine(1,1); break;  // 1층 상품 박스 꺼내기 루틴
    case 31: PlaceRoutine(1,1); break;   // 1층 상품 박스 넣기 루틴
    case 32: PickupRoutine(2,1); break;  // 2층 상품 박스 꺼내기 루틴
    case 33: PlaceRoutine(2,1); break;   // 2층 상품 박스 넣기 루틴
    case 34: PickupRoutine(4,1); break;  // 3층 상품 박스 꺼내기 루틴
    case 35: PlaceRoutine(4,1); break;   // 3층 상품 박스 넣기 루틴
    case 36: PickupRoutine(2,2); break;  // 고객 박스 꺼내기 루틴
    case 37: PlaceRoutine(2,2); break;   // 고객 박스 넣기 루틴
  }
}

// === 루틴 ===
void PickupRoutine(int floor, int target) {     // 박스 꺼내기 루틴
  moveToFloor(floor);

  delay(500);
  digitalWrite(ELEV_DIR,LOW);
    for (int i = 0; i < 100; i++) {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay);
  }
  
  if (target == 1) {
    currentCommand = 6; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, LOW); delay(500);
    currentCommand = 5; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, HIGH);
    moveToFloor(2); Serial.write(210);
  } else if (target == 2) {
      digitalWrite(ELEV_DIR,LOW);
    for (int i = 0; i < 200; i++) {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay);
  }
    currentCommand = 8; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, LOW); delay(500);
    currentCommand = 7; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH);
    moveToFloor(2); Serial.write(211);
  }
  Serial.println("꺼내기 루틴 완료됨");
}

void PlaceRoutine(int floor, int target) {    // 박스 넣기
  moveToFloor(floor);
  delay(500);
  digitalWrite(ELEV_DIR,HIGH);
    for (int i = 0; i < 100; i++) {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay);
  }
  if (target == 1) {
     digitalWrite(L_solenoid, LOW); delay(200);
    currentCommand = 6; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, HIGH); delay(500);
    currentCommand = 5; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH);
    Serial.write(212);
  } else if (target == 2) {
    digitalWrite(ELEV_DIR,HIGH);
    for (int i = 0; i < 100; i++) {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay);
  }
  digitalWrite(R_solenoid, LOW); delay(200);
    currentCommand = 8; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH); delay(500);
    currentCommand = 7; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH);
    Serial.write(213);
  }
  Serial.println("내리기 루틴 완료됨");
}

// === 모터 제어 ===
void checkAndRunMotor() {       // Y축 모터 제어 
  switch (currentCommand) {
    case 5: digitalWrite(Y_LEFT_DIR, HIGH); while (getFilteredYState(0) == HIGH) { generatePulse(Y_LEFT_PUL, frequency); updateAndPrintYSensors(); } break;
    case 6: digitalWrite(Y_LEFT_DIR, LOW);  while (getFilteredYState(1) == HIGH) { generatePulse(Y_LEFT_PUL, frequency); updateAndPrintYSensors(); } break;
    case 7: digitalWrite(Y_RIGHT_DIR, HIGH); while (getFilteredYState(2) == HIGH) { generatePulse(Y_RIGHT_PUL, frequency); updateAndPrintYSensors(); } break;
    case 8: digitalWrite(Y_RIGHT_DIR, LOW);  while (getFilteredYState(3) == HIGH) { generatePulse(Y_RIGHT_PUL, frequency); updateAndPrintYSensors(); } break;
  }
  Serial.println("모터 동작 완료");
  motorRunning = false;
}

// === 엘리베이터 이동 ===
void moveToFloor(int targetFloor) {     // z축 모터 제어
  if (targetFloor == currentFloor) return;
  isMovingElevator = true;
  digitalWrite(ELEV_DIR, (targetFloor > currentFloor) ? HIGH : LOW);
  while (isMovingElevator) {
    generateElevPulse(); updateCurrentFloor();
    if (currentFloor == targetFloor) {
      isMovingElevator = false;
      Serial.print("도착 층: "); Serial.println(currentFloor);
      Serial.write(100);
    }
  }
}

void generateElevPulse() {     // Z축 펄스 생성
  int period_us = 1000000 / elevFrequency;
  digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(period_us / 2);
  digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(period_us / 2);
}

void generatePulse(int pin, int freq) {     // Y축 펄스 생성
  int period = 1000000 / freq;
  digitalWrite(pin, HIGH); delayMicroseconds(period / 2);
  digitalWrite(pin, LOW);  delayMicroseconds(period / 2);
}

// === 센서 처리 ===
void updateCurrentFloor() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(floorSensorPins[i]) == LOW) {
      if (currentFloor != i + 1) {
        currentFloor = i + 1;
        Serial.print("현재 층: "); Serial.println(currentFloor);
      }
      break;
    }
  }
}

void updateAndPrintYSensors() {
  for (int i = 0; i < 4; i++) {
    int raw = digitalRead(ySensorPins[i]);
    float signal = (raw == LOW) ? 0.0 : 1.0;
    filteredY[i] = alpha * filteredY[i] + (1 - alpha) * signal;
    int filtered = (filteredY[i] < threshold) ? LOW : HIGH;

    if (raw != prevRawY[i]) {
      printTimestamp(); Serial.print("[원본] "); Serial.print(yLabels[i]); Serial.print(" 센서: ");
      Serial.println(raw == LOW ? "감지됨" : "해제됨");
      prevRawY[i] = raw;
    }
    if (filtered != prevFilteredY[i]) {
      printTimestamp(); Serial.print("[필터] "); Serial.print(yLabels[i]); Serial.print(" 센서: ");
      Serial.println(filtered == LOW ? "감지됨" : "해제됨");
      prevFilteredY[i] = filtered;
    }
  }
}

int getFilteredYState(int i) {
  return (filteredY[i] < threshold) ? LOW : HIGH;
}

// === 기타 유틸 ===


void stopAllMotors() {
  digitalWrite(Y_LEFT_PUL, LOW);
  digitalWrite(Y_RIGHT_PUL, LOW);
  digitalWrite(ELEV_PUL, LOW);
  Serial.println("모터 정지됨.");
}

void setBothPanels(uint8_t r, uint8_t g, uint8_t b) {   // 네오픽셀 제어
  for (int i = 0; i < NUMPIXELS; i++) {
    leftStrip.setPixelColor(i, leftStrip.Color(r, g, b));
    rightStrip.setPixelColor(i, rightStrip.Color(r, g, b));
  }
  leftStrip.show(); rightStrip.show();
}

void turnOffPanels() {    // 네오픽셀 끄기
  setBothPanels(0, 0, 0);
}

void printTimestamp() {
  unsigned long ms = millis();
  unsigned int sec = ms / 1000;
  unsigned int min = sec / 60;
  sec = sec % 60;
  Serial.print("[");
  if (min < 10) Serial.print("0"); Serial.print(min);
  Serial.print(":");
  if (sec < 10) Serial.print("0"); Serial.print(sec);
  Serial.print("] ");
}
