const int PUL = 10;   // 펄스 신호 핀
const int DIR = 13;   // 방향 제어 핀
const int frequency = 2000; // 펄스 주파수 (Hz)
const int pin7 = 7;   // 정지 조건 핀 (전진 중)
const int pin8 = 8;   // 정지 조건 핀 (후진 중)

bool motorRunning = false; // 모터 동작 여부 저장
char currentCommand = '0'; // 현재 명령 저장

void setup() {
    pinMode(PUL, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(pin7, INPUT);
    pinMode(pin8, INPUT);
    Serial.begin(9600);
}

void loop() {
    if (Serial.available() > 0) {  
        char command = Serial.read(); 

        if (command == '1' || command == '2' || command == '0') {
            currentCommand = command; // 현재 명령 저장
            motorRunning = (command != '0'); // '0'이면 정지 상태 유지
        }
    }

    if (motorRunning) {
        if (currentCommand == '1') {
            Serial.println("전진 중...");
            digitalWrite(DIR, HIGH);
            while (digitalRead(pin7) == HIGH) { // 7번 핀이 LOW가 될 때까지 실행
                generatePulse();
            }
            Serial.println("start");
            motorRunning = false; // 정지 상태 유지
        } 
        else if (currentCommand == '2') {
            Serial.println("후진 중...");
            digitalWrite(DIR, LOW);
            while (digitalRead(pin8) == HIGH) { // 8번 핀이 LOW가 될 때까지 실행
                generatePulse();
            }
            Serial.println("end");
            motorRunning = false; // 정지 상태 유지
        }
    } else {
        digitalWrite(PUL, LOW); // 모터 완전 정지
    }
}

// 펄스 생성 함수
void generatePulse() {
    int period_us = 1000000 / frequency;
    digitalWrite(PUL, HIGH);
    delayMicroseconds(period_us / 2);
    digitalWrite(PUL, LOW);
    delayMicroseconds(period_us / 2);
}
