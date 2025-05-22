const int ELEV_PUL = 11;
const int ELEV_DIR = 10;

const int step_up = 3000;
const int step_down = 3000;
const int pulse_delay = 500; // 마이크로초

void setup() {
  pinMode(ELEV_PUL, OUTPUT);
  pinMode(ELEV_DIR, OUTPUT);
  delay(500);

  digitalWrite(ELEV_PUL, LOW);
  digitalWrite(ELEV_DIR, LOW);
  delay(500);


  digitalWrite(ELEV_DIR, LOW);
  moveSteps(1000);
  

  // 5cm 하강
  //digitalWrite(ELEV_DIR, LOW);
  //moveSteps(step_down);
}

void loop() {
  // 반복 없음
}

void moveSteps(int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay);
  }
}
