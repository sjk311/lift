#include <Adafruit_NeoPixel.h>

// === 핀 설정 ===
const int floorSensorPins[4] = {4, 5, 6, 7};           // 1~4층 센서
const int zSensorPins[4] = {22, 24, 26, 28};           // Z축 센서

const int Y_LEFT_PUL = 13;
const int Y_LEFT_DIR = 12;
const int Y_RIGHT_PUL = 11;
const int Y_RIGHT_DIR = 10;
const int ELEV_PUL = 9;
const int ELEV_DIR = 8;
const int L_solenoid = 32;
const int R_solenoid = 34;

// === 레이블 ===
const char* floorLabels[4] = {"1층", "2층", "3층", "4층"};
const char* zLabels[4] = {"왼쪽 BACK", "왼쪽 FRONT", "오른쪽 BACK", "오른쪽 FRONT"};

// === 필터 관련 ===
float filteredFloor[4] = {1, 1, 1, 1};
float filteredZ[4] = {1, 1, 1, 1};
int prevRawFloor[4] = {HIGH, HIGH, HIGH, HIGH};
int prevFilteredFloor[4] = {HIGH, HIGH, HIGH, HIGH};
int prevRawZ[4] = {HIGH, HIGH, HIGH, HIGH};
int prevFilteredZ[4] = {HIGH, HIGH, HIGH, HIGH};

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

// === 네오픽셀 설정 ===
#define LEFT_LED_PIN   48
#define RIGHT_LED_PIN  47
#define NUMPIXELS      12

Adafruit_NeoPixel leftStrip(NUMPIXELS, LEFT_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightStrip(NUMPIXELS, RIGHT_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    pinMode(floorSensorPins[i], INPUT_PULLUP);
    pinMode(zSensorPins[i], INPUT_PULLUP);
  }

  pinMode(Y_LEFT_PUL, OUTPUT);  pinMode(Y_LEFT_DIR, OUTPUT);
  pinMode(Y_RIGHT_PUL, OUTPUT); pinMode(Y_RIGHT_DIR, OUTPUT);
  pinMode(ELEV_PUL, OUTPUT);   pinMode(ELEV_DIR, OUTPUT);
  pinMode(L_solenoid, OUTPUT); pinMode(R_solenoid, OUTPUT);

  leftStrip.begin();
  rightStrip.begin();
  leftStrip.show();
  rightStrip.show();
  setBothPanels(0, 0, 0);

  Serial.println("=== 시스템 시작됨 ===");
}


void loop() {
  updateAndPrintFloorSensors();
  updateAndPrintZSensors();

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    motorRunning = (input.toInt() != 0);

    // LED 제어 명령
    if (input == "r") {
      setBothPanels(255, 0, 0); Serial.println("LED: 빨강");
    } else if (input == "g") {
      setBothPanels(0, 255, 0); Serial.println("LED: 초록");
    } else if (input == "b") {
      setBothPanels(0, 0, 255); Serial.println("LED: 파랑");
    } else if (input == "y") {
      setBothPanels(255, 255, 0); Serial.println("LED: 노랑");
    } else if (input == "off") {
      turnOffPanels(); Serial.println("LED: OFF");

    // 엘리베이터 수동 제어 
    } else if (input.startsWith("u ") || input.startsWith("d ")) {
      int spaceIndex = input.indexOf(' ');
      String direction = input.substring(0, spaceIndex);
      int cntLimit = input.substring(spaceIndex + 1).toInt();

      if (cntLimit <= 0) {
        Serial.println("잘못된 펄스 수입니다.");
      } else {
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

    // 나머지 기존 명령 처리
    } else {
      currentCommand = input.toInt();

      switch (currentCommand) {
        case 0:
          stopAllMotors();
          break;

        case 1: case 2: case 3: case 4:
          moveToFloor(currentCommand);
          break;

        case 10:
          digitalWrite(L_solenoid, HIGH);
          Serial.println("L_solenoid ON");
          break;

        case 11:
          digitalWrite(L_solenoid, LOW);
          Serial.println("L_solenoid OFF");
          break;

        case 12:
          digitalWrite(R_solenoid, HIGH);
          Serial.println("R_solenoid ON");
          break;

        case 13:
          digitalWrite(R_solenoid, LOW);
          Serial.println("R_solenoid OFF");
          break;

        case 20: {
          digitalWrite(ELEV_DIR, HIGH);
          for (int cnt = 0; cnt < 150; cnt++) {
            digitalWrite(ELEV_PUL, HIGH);
            delayMicroseconds(pulse_delay);
            digitalWrite(ELEV_PUL, LOW);
            delayMicroseconds(pulse_delay);
          }
          break;
        }

        case 21: {
          digitalWrite(ELEV_DIR, LOW);
          for (int cnt = 0; cnt < 150; cnt++) {
            digitalWrite(ELEV_PUL, HIGH);
            delayMicroseconds(pulse_delay);
            digitalWrite(ELEV_PUL, LOW);
            delayMicroseconds(pulse_delay);
          }
          break;
        }
      }
    }
  }

  if (motorRunning) checkAndRunMotor();
  delay(10);
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

// === 모터 실행 ===
void checkAndRunMotor() {
  switch (currentCommand) {
    case 5:
      Serial.println("Y축 왼쪽 하강 중...");
      digitalWrite(Y_LEFT_DIR, HIGH);
      while (getFilteredZState(0) == HIGH) {
        generatePulse(Y_LEFT_PUL, frequency);
        updateAndPrintZSensors();
      }
      break;
    case 6:
      Serial.println("Y축 왼쪽 상승 중...");
      digitalWrite(Y_LEFT_DIR, LOW);
      while (getFilteredZState(1) == HIGH) {
        generatePulse(Y_LEFT_PUL, frequency);
        updateAndPrintZSensors();
      }
      break;
    case 7:
      Serial.println("Y축 오른쪽 하강 중...");
      digitalWrite(Y_RIGHT_DIR, HIGH);
      while (getFilteredZState(2) == HIGH) {
        generatePulse(Y_RIGHT_PUL, frequency);
        updateAndPrintZSensors();
      }
      break;
    case 8:
      Serial.println("Y축 오른쪽 상승 중...");
      digitalWrite(Y_RIGHT_DIR, LOW);
      while (getFilteredZState(3) == HIGH) {
        generatePulse(Y_RIGHT_PUL, frequency);
        updateAndPrintZSensors();
      }
      break;
  }

  Serial.println("모터 동작 완료");
  motorRunning = false;
}

// === 엘리베이터 이동 ===
void moveToFloor(int targetFloor) {
  if (targetFloor == currentFloor) {
    Serial.println("이미 해당 층입니다.");
    return;
  }

  isMovingElevator = true;
  digitalWrite(ELEV_DIR, (targetFloor > currentFloor) ? HIGH : LOW);
  Serial.println(targetFloor > currentFloor ? "엘리베이터 상승 중..." : "엘리베이터 하강 중...");

  while (isMovingElevator) {
    generatePulse(ELEV_PUL, elevFrequency);
    updateAndPrintFloorSensors();

    for (int i = 0; i < 4; i++) {
      if ((filteredFloor[i] < threshold) && currentFloor != (i + 1)) {
        currentFloor = i + 1;
        Serial.print("도착 층: "); Serial.println(currentFloor);
        if (currentFloor == targetFloor) {
          isMovingElevator = false;
          break;
        }
      }
    }
  }

  Serial.println("엘리베이터 정지");
}

// === 센서 EMA 출력 ===
void updateAndPrintFloorSensors() {
  for (int i = 0; i < 4; i++) {
    int raw = digitalRead(floorSensorPins[i]);
    float signal = (raw == LOW) ? 0.0 : 1.0;
    filteredFloor[i] = alpha * filteredFloor[i] + (1 - alpha) * signal;
    int filtered = (filteredFloor[i] < threshold) ? LOW : HIGH;

    if (raw != prevRawFloor[i]) {
      printTimestamp(); Serial.print("[원본] "); Serial.print(floorLabels[i]); Serial.print(" 센서: ");
      Serial.println(raw == LOW ? "감지됨" : "해제됨");
      prevRawFloor[i] = raw;
    }

    if (filtered != prevFilteredFloor[i]) {
      printTimestamp(); Serial.print("[필터] "); Serial.print(floorLabels[i]); Serial.print(" 센서: ");
      Serial.println(filtered == LOW ? "감지됨" : "해제됨");
      prevFilteredFloor[i] = filtered;
    }
  }
}

void updateAndPrintZSensors() {
  for (int i = 0; i < 4; i++) {
    int raw = digitalRead(zSensorPins[i]);
    float signal = (raw == LOW) ? 0.0 : 1.0;
    filteredZ[i] = alpha * filteredZ[i] + (1 - alpha) * signal;
    int filtered = (filteredZ[i] < threshold) ? LOW : HIGH;

    if (raw != prevRawZ[i]) {
      printTimestamp(); Serial.print("[원본] "); Serial.print(zLabels[i]); Serial.print(" 센서: ");
      Serial.println(raw == LOW ? "감지됨" : "해제됨");
      prevRawZ[i] = raw;
    }

    if (filtered != prevFilteredZ[i]) {
      printTimestamp(); Serial.print("[필터] "); Serial.print(zLabels[i]); Serial.print(" 센서: ");
      Serial.println(filtered == LOW ? "감지됨" : "해제됨");
      prevFilteredZ[i] = filtered;
    }
  }
}

// === 필터 상태 가져오기 ===
int getFilteredZState(int i) {
  return (filteredZ[i] < threshold) ? LOW : HIGH;
}

// === 펄스 생성 ===
void generatePulse(int pin, int freq) {
  int period = 1000000 / freq;
  digitalWrite(pin, HIGH); delayMicroseconds(period / 2);
  digitalWrite(pin, LOW);  delayMicroseconds(period / 2);
}

// === 전체 모터 정지 ===
void stopAllMotors() {
  digitalWrite(Y_LEFT_PUL, LOW);
  digitalWrite(Y_RIGHT_PUL, LOW);
  digitalWrite(ELEV_PUL, LOW);
  Serial.println("모터 정지됨.");
}

// === 타임스탬프 출력 ===
void printTimestamp() {
  unsigned long ms = millis();
  unsigned int sec = ms / 1000;
  unsigned int min = sec / 60;
  sec = sec % 60;

  Serial.print("[");
  if (min < 10) Serial.print("0");
  Serial.print(min);
  Serial.print(":");
  if (sec < 10) Serial.print("0");
  Serial.print(sec);
  Serial.print("] ");
}
