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
#define WHITE_LED_PIN 41
#define NUMPIXELS 12

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
  pinMode(WHITE_LED_PIN, OUTPUT);
  digitalWrite(WHITE_LED_PIN, LOW);  // 초기 상태 OFF

  leftStrip.begin(); rightStrip.begin();
  leftStrip.show(); rightStrip.show();


  setBothPanels(0, 0, 0);   // 초기 led off

  
  digitalWrite(R_solenoid, HIGH);
  digitalWrite(L_solenoid, HIGH);
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
  else if (input == "on") 
  {
    turnOnWhite();
    Serial.println("1");
  }
  else if (input == "off") 
  {
    turnOffPanels();
    turnOffWhite();
    Serial.println("1");
  }
  else if (input.startsWith("u ") || input.startsWith("d ")) handleManualElevator(input);
  else handleCommand(input.toInt());
}

/*
void handleManualElevator(String input) {
  int spaceIndex = input.indexOf(' ');
  String direction = input.substring(0, spaceIndex);
  int cntLimit = input.substring(spaceIndex + 1).toInt();

  if (cntLimit <= 0) return;
  

  digitalWrite(ELEV_DIR, (direction == "u") ? HIGH : LOW);
  for (int i = 0; i < cntLimit; i++) {
    digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW); delayMicroseconds(pulse_delay);
  }
  Serial.println("1");
}
*/

void handleManualElevator(String input) {
  int spaceIndex = input.indexOf(' ');
  String direction = input.substring(0, spaceIndex);
  int cntLimit = input.substring(spaceIndex + 1).toInt();

  if (cntLimit <= 0) return;

  digitalWrite(ELEV_DIR, (direction == "u") ? HIGH : LOW);

  // 리프트 동작 중 깜빡이기용 변수
  unsigned long lastToggleTime = millis();
  bool panelOn = false;

  for (int i = 0; i < cntLimit; i++) {
    digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW); delayMicroseconds(pulse_delay);

    // 0.5초마다 노란색 패널 깜빡이기
    if (millis() - lastToggleTime >= 500) {
      panelOn = !panelOn;
      if (panelOn) {
        setBothPanels(255, 255, 0);  // 노란색 ON
      } else {
        setBothPanels(0, 0, 0);      // OFF
      }
      lastToggleTime = millis();
    }
  }

  // 리프트 동작 후 패널 OFF
  if(direction == "d"){
    setBothPanels(0, 0, 0);
  }
  Serial.println("1");
}


void handleCommand(int cmd) {
  currentCommand = cmd;
  switch (cmd) {
    case 0: stopAllMotors(); break;
    case 1: case 2: case 3: case 4: moveToFloor(cmd); break;
    case 10: digitalWrite(L_solenoid, HIGH); break;
    case 11: digitalWrite(L_solenoid, LOW); break;
    case 12: digitalWrite(R_solenoid, HIGH); break;
    case 13: digitalWrite(R_solenoid, LOW); break;
    case 30: PickupRoutine(1,1); break;
    case 31: PlaceRoutine(1,1); break;
    case 32: PickupRoutine(2,1); break;
    case 33: PlaceRoutine(2,1); break;
    case 34: PickupRoutine(4,1); break;
    case 35: PlaceRoutine(4,1); break;
    case 40: PickupRoutine(1,2); break;
    case 41: PlaceRoutine(1,2); break;
    case 42: PickupRoutine(2,2); break;
    case 43: PlaceRoutine(2,2); break;
  }
}

// === 루틴 ===

void PickupRoutine(int floor, int target) {
  setBothPanels(255, 255, 0);
  moveToFloor(floor);
  delay(500);

  digitalWrite(ELEV_DIR, LOW);
  for (int i = 0; i < 100; i++) {
    digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
  }

  if (target == 1) {
    currentCommand = 6; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, LOW); delay(500);
    currentCommand = 5; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, HIGH);

    digitalWrite(ELEV_DIR, HIGH);
    for (int i = 0; i < 100; i++) {
      digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
      digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
    }

    moveToFloor(2);
    turnOnWhite();
    delay(2500);
    Serial.println("1");
  }

  else if (target == 2) {
    
    digitalWrite(ELEV_DIR, LOW);
    for (int i = 0; i < 100; i++) {
      digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
      digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
    }
    

    currentCommand = 8; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, LOW); delay(500);
    currentCommand = 7; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH);

    digitalWrite(ELEV_DIR, HIGH);
    for (int i = 0; i < 300; i++) {
      digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
      digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
    }

    moveToFloor(2);
    Serial.println("1");
  }
  turnOffPanels();
}


void PlaceRoutine(int floor, int target) {
  turnOffWhite();
  setBothPanels(135, 55, 200);
  moveToFloor(floor);
  delay(500);

  digitalWrite(ELEV_DIR, HIGH);
  for (int i = 0; i < 150; i++) {
    digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
  }


  if (target == 1) {
    //digitalWrite(L_solenoid, LOW); delay(200);
    currentCommand = 6; motorRunning = true; checkAndRunMotor();
    //digitalWrite(L_solenoid, HIGH); delay(2000);
    currentCommand = 5; motorRunning = true; checkAndRunMotor();
    //digitalWrite(R_solenoid, HIGH);

    digitalWrite(ELEV_DIR, LOW);
    for (int i = 0; i < 100; i++) {
      digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
      digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
    }

    Serial.println("1");
  }

  else if (target == 2) {
    digitalWrite(ELEV_DIR, HIGH);
    for (int i = 0; i < 100; i++) {
      digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
      digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
    }

    digitalWrite(R_solenoid, LOW); delay(200);
    currentCommand = 8; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH); delay(500);
    currentCommand = 7; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH);

    digitalWrite(ELEV_DIR, LOW);
    for (int i = 0; i < 200; i++) {
      digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
      digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
    }

    Serial.println("1");
  }
  moveToFloor(2);
  turnOffPanels();
}


/*
void PickupRoutine(int floor, int target) {
  moveToFloor(floor);
  delay(500);

  if (target == 1) {
    if (floor == 2){
      digitalWrite(ELEV_DIR, LOW);
      for (int i = 0; i < 200; i++) {
        digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
        digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
      }
    }
    currentCommand = 6; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, LOW); delay(500);
    currentCommand = 5; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, HIGH);

    moveToFloor(2);
    if (floor == 2){
      digitalWrite(ELEV_DIR, HIGH);
      for (int i = 0; i < 200; i++) {
        digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(pulse_delay);
        digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(pulse_delay);
      }
    }
    
    Serial.println("1");
  }

  else if (target == 2) {

    currentCommand = 8; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, LOW); delay(500);
    currentCommand = 7; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH);

    moveToFloor(2);
    Serial.println("1");
  }
}


void PlaceRoutine(int floor, int target) {
  moveToFloor(floor);
  delay(500);


  if (target == 1) {
    digitalWrite(L_solenoid, LOW); delay(200);
    currentCommand = 6; motorRunning = true; checkAndRunMotor();
    digitalWrite(L_solenoid, HIGH); delay(500);
    currentCommand = 5; motorRunning = true; checkAndRunMotor();
    


    Serial.println("1");
  }

  else if (target == 2) {


    digitalWrite(R_solenoid, LOW); delay(200);
    currentCommand = 8; motorRunning = true; checkAndRunMotor();
    digitalWrite(R_solenoid, HIGH); delay(500);
    currentCommand = 7; motorRunning = true; checkAndRunMotor();
    

    Serial.println("1");
  }
}
*/

// === 모터 제어 ===
void checkAndRunMotor() {
  switch (currentCommand) {
    case 5: digitalWrite(Y_LEFT_DIR, HIGH); while (getFilteredYState(0) == HIGH) { generatePulse(Y_LEFT_PUL, frequency); updateAndPrintYSensors(); } break;
    case 6: digitalWrite(Y_LEFT_DIR, LOW);  while (getFilteredYState(1) == HIGH) { generatePulse(Y_LEFT_PUL, frequency); updateAndPrintYSensors(); } break;
    case 7: digitalWrite(Y_RIGHT_DIR, HIGH); while (getFilteredYState(2) == HIGH) { generatePulse(Y_RIGHT_PUL, frequency); updateAndPrintYSensors(); } break;
    case 8: digitalWrite(Y_RIGHT_DIR, LOW);  while (getFilteredYState(3) == HIGH) { generatePulse(Y_RIGHT_PUL, frequency); updateAndPrintYSensors(); } break;
  }
  motorRunning = false;
}

// === 엘리베이터 이동 ===
void moveToFloor(int targetFloor) {
  if (targetFloor == currentFloor) return;
  isMovingElevator = true;
  digitalWrite(ELEV_DIR, (targetFloor > currentFloor) ? HIGH : LOW);
  while (isMovingElevator) {
    generateElevPulse(); updateCurrentFloor();
    if (currentFloor == targetFloor) isMovingElevator = false;
  }
}

void generateElevPulse() {
  int period_us = 1000000 / elevFrequency;
  digitalWrite(ELEV_PUL, HIGH); delayMicroseconds(period_us / 2);
  digitalWrite(ELEV_PUL, LOW);  delayMicroseconds(period_us / 2);
}

void generatePulse(int pin, int freq) {
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
    prevRawY[i] = raw;
    prevFilteredY[i] = filtered;
  }
}

int getFilteredYState(int i) {
  return (filteredY[i] < threshold) ? LOW : HIGH;
}

// === 유틸 ===
void stopAllMotors() {
  digitalWrite(Y_LEFT_PUL, LOW);
  digitalWrite(Y_RIGHT_PUL, LOW);
  digitalWrite(ELEV_PUL, LOW);
}

void setBothPanels(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    leftStrip.setPixelColor(i, leftStrip.Color(r, g, b));
    rightStrip.setPixelColor(i, rightStrip.Color(r, g, b));
  }
  leftStrip.show(); rightStrip.show();
}

void turnOffPanels() {
  setBothPanels(0, 0, 0);
}

void turnOnWhite() {
  digitalWrite(WHITE_LED_PIN, HIGH);
}

void turnOffWhite() {
  digitalWrite(WHITE_LED_PIN, LOW);
}



void printTimestamp() {}  // 출력 제거용 빈 함수
