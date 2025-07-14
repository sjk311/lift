const int ELEV_PUL = 9;
const int ELEV_DIR = 8;

const int pulse_delay = 1000; // 마이크로초
int cnt = 0;

void setup() {
  pinMode(ELEV_PUL, OUTPUT);
  pinMode(ELEV_DIR, OUTPUT);
  delay(3000);

  //digitalWrite(ELEV_DIR, LOW);
  digitalWrite(ELEV_DIR, HIGH);
  while (1)
  {
    digitalWrite(ELEV_PUL, HIGH);
    delayMicroseconds(pulse_delay);
    digitalWrite(ELEV_PUL, LOW);
    delayMicroseconds(pulse_delay); 
    cnt ++;
    if (cnt == 150){
      break;
    }

  
  }
  //moveSteps(500);
  
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
