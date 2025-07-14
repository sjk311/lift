const int floorSensorPins[4] = {4, 5, 6, 7};  // 1~4층
const int zSensorPins[4] = {22, 24, 26, 28}; // Z축 센서

// EMA 필터 관련 변수
float filteredFloor[4] = {1.0, 1.0, 1.0, 1.0};
float filteredZ[4]     = {1.0, 1.0, 1.0, 1.0};
const float alpha = 0.9;
const float threshold = 0.5;

// 상태 기록 (변화 감지용)
int prevRawFloor[4] = {HIGH, HIGH, HIGH, HIGH};
int prevFilteredFloor[4] = {HIGH, HIGH, HIGH, HIGH};

int prevRawZ[4] = {HIGH, HIGH, HIGH, HIGH};
int prevFilteredZ[4] = {HIGH, HIGH, HIGH, HIGH};

// 센서 레이블
const char* floorLabels[4] = {"1층", "2층", "3층", "4층"};
const char* zLabels[4] = {"왼쪽 BACK", "왼쪽 FRONT", "오른쪽 BACK", "오른쪽 FRONT"};

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) {
    pinMode(floorSensorPins[i], INPUT_PULLUP);
    pinMode(zSensorPins[i], INPUT_PULLUP);
  }
  Serial.println("=== 필터 적용 전/후 비교 시작 ===");
}

void loop() {
  unsigned long now = millis();

  // 층 센서
  for (int i = 0; i < 4; i++) {
    int raw = digitalRead(floorSensorPins[i]);
    float signal = (raw == LOW) ? 0.0 : 1.0;
    filteredFloor[i] = alpha * filteredFloor[i] + (1 - alpha) * signal;
    int filtered = (filteredFloor[i] > threshold) ? HIGH : LOW;

    // 원본 센서 출력 변화 감지
    if (raw != prevRawFloor[i]) {
      printTimestamp();
      Serial.print("[원본] ");
      Serial.print(floorLabels[i]);
      Serial.print(" 센서: ");
      Serial.println(raw == LOW ? "감지됨" : "해제됨");
      prevRawFloor[i] = raw;
    }

    // 필터 센서 출력 변화 감지
    if (filtered != prevFilteredFloor[i]) {
      printTimestamp();
      Serial.print("[필터     1] ");
      Serial.print(floorLabels[i]);
      Serial.print(" 센서: ");
      Serial.println(filtered == LOW ? "감지됨" : "해제됨");
      prevFilteredFloor[i] = filtered;
    }
  }

  // Z축 센서
  for (int i = 0; i < 4; i++) {
    int raw = digitalRead(zSensorPins[i]);
    float signal = (raw == LOW) ? 0.0 : 1.0;
    filteredZ[i] = alpha * filteredZ[i] + (1 - alpha) * signal;
    int filtered = (filteredZ[i] > threshold) ? HIGH : LOW;

    if (raw != prevRawZ[i]) {
      printTimestamp();
      Serial.print("[원본] ");
      Serial.print(zLabels[i]);
      Serial.print(" 센서: ");
      Serial.println(raw == LOW ? "감지됨" : "해제됨");
      prevRawZ[i] = raw;
    }

    if (filtered != prevFilteredZ[i]) {
      printTimestamp();
      Serial.print("[필터       1] ");
      Serial.print(zLabels[i]);
      Serial.print(" 센서: ");
      Serial.println(filtered == LOW ? "감지됨" : "해제됨");
      prevFilteredZ[i] = filtered;
    }
  }

  delay(1); // 과부하 방지용 최소 딜레이
}

void printTimestamp() {
  unsigned long ms = millis();
  unsigned int totalSeconds = ms / 1000;
  unsigned int minutes = totalSeconds / 60;
  unsigned int seconds = totalSeconds % 60;

  Serial.print("[");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
  Serial.print("] ");
}