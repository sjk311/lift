const int sensorPins[4] = {4, 5, 6, 7}; // 1~4층 센서

// UNO R4에 맞게 핀 번호 변경
const int Y_LEFT_SENSOR_BACK = 8;
const int Y_LEFT_SENSOR_FRONT = 9;
const int Y_RIGHT_SENSOR_BACK = 10;
const int Y_RIGHT_SENSOR_FRONT = 11;

// 이전 상태 저장용 변수
int prevFloorState[4] = {HIGH, HIGH, HIGH, HIGH};
int prevLeftBack = HIGH;
int prevLeftFront = HIGH;
int prevRightBack = HIGH;
int prevRightFront = HIGH;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  pinMode(Y_LEFT_SENSOR_BACK, INPUT_PULLUP);
  pinMode(Y_LEFT_SENSOR_FRONT, INPUT_PULLUP);
  pinMode(Y_RIGHT_SENSOR_BACK, INPUT_PULLUP);
  pinMode(Y_RIGHT_SENSOR_FRONT, INPUT_PULLUP);

  Serial.println("센서 변경 감지 시작");
}

void loop() {
  // 층 센서 확인
  for (int i = 0; i < 4; i++) {
    int currentState = digitalRead(sensorPins[i]);
    if (currentState != prevFloorState[i]) {
      Serial.print(i + 1);
      Serial.print("층 센서: ");
      Serial.println(currentState == LOW ? "감지됨" : "해제됨");
      prevFloorState[i] = currentState;
    }
  }

  // Z축 방향 센서 확인
  checkSensor(Y_LEFT_SENSOR_BACK, prevLeftBack, "왼쪽 BACK 센서");
  checkSensor(Y_LEFT_SENSOR_FRONT, prevLeftFront, "왼쪽 FRONT 센서");
  checkSensor(Y_RIGHT_SENSOR_BACK, prevRightBack, "오른쪽 BACK 센서");
  checkSensor(Y_RIGHT_SENSOR_FRONT, prevRightFront, "오른쪽 FRONT 센서");

  delay(10); // 빠른 루프 방지
}

void checkSensor(int pin, int &prevState, const char* label) {
  int currentState = digitalRead(pin);
  if (currentState != prevState) {
    Serial.print(label);
    Serial.print(": ");
    Serial.println(currentState == LOW ? "감지됨" : "해제됨");
    prevState = currentState;
  }
}
