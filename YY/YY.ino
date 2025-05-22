const int Y_LEFT_PUL = 13;   // 펄스 신호 핀
const int Y_LEFT_DIR = 12;   // 방향 제어 핀
const int Y_RIGHT_PUL = 11;   // 펄스 신호 핀
const int Y_RIGHT_DIR = 10;   // 방향 제어 핀

const int frequency = 2000; // 펄스 주파수 (Hz)

const int Y_LEFT_SENSOR_BOTTOM = 22;   // 정지 조건 핀 하단
const int Y_LEFT_SENSOR_TOP = 24;   // 정지 조건 핀 상단
const int Y_RIGHT_SENSOR_BOTTOM = 26;   // 정지 조건 핀 하단
const int Y_RIGHT_SENSOR_TOP = 28;   // 정지 조건 핀 상단

bool motorRunning = false; // 모터 동작 여부 저장
char currentCommand = '0'; // 현재 명령 저장

void setup() {
    pinMode(Y_LEFT_PUL, OUTPUT);
    pinMode(Y_LEFT_DIR, OUTPUT);
    pinMode(Y_LEFT_SENSOR_BOTTOM, INPUT);
    pinMode(Y_LEFT_SENSOR_TOP, INPUT);
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
        if (currentCommand == '5') {
            Serial.println("Z축 왼쪽 하강 중...");
            digitalWrite(Y_LEFT_DIR, HIGH);
            while (digitalRead(Y_LEFT_SENSOR_BOTTOM) == HIGH) { 
                generateY_LEFT_PULse();
            }
            Serial.println("bottom");
            motorRunning = false; // 정지 상태 유지
        } 
        else if (currentCommand == '6') {
            Serial.println("Z축 왼쪽 상승 중...");
            digitalWrite(Y_LEFT_DIR, LOW);
            while (digitalRead(Y_LEFT_SENSOR_TOP) == HIGH) { 
                generateY_LEFT_PULse();
            }
            Serial.println("top");
            motorRunning = false; // 정지 상태 유지
        }
    } else {
        digitalWrite(Y_LEFT_PUL, LOW); // 모터 완전 정지
    }
}

// 펄스 생성 함수
void generateY_LEFT_PULse() {
    int period_us = 1000000 / frequency;
    digitalWrite(Y_LEFT_PUL, HIGH);
    delayMicroseconds(period_us / 2);
    digitalWrite(Y_LEFT_PUL, LOW);
    delayMicroseconds(period_us / 2);
}

void generateY_RIGHT_PULse() {
    int period_us = 1000000 / frequency;
    digitalWrite(Y_LEFT_PUL, HIGH);
    delayMicroseconds(period_us / 2);
    digitalWrite(Y_LEFT_PUL, LOW);
    delayMicroseconds(period_us / 2);
}
