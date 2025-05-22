const int Y_LEFT_PUL = 13;   // 왼쪽 펄스 핀
const int Y_LEFT_DIR = 12;   // 왼쪽 방향 핀
const int Y_RIGHT_PUL = 11;  // 오른쪽 펄스 핀
const int Y_RIGHT_DIR = 10;  // 오른쪽 방향 핀

const int frequency = 2000; // 펄스 주파수 (Hz)

const int Y_LEFT_SENSOR_BOTTOM = 22;
const int Y_LEFT_SENSOR_TOP = 24;
const int Y_RIGHT_SENSOR_BOTTOM = 26;
const int Y_RIGHT_SENSOR_TOP = 28;

bool motorRunning = false;
char currentCommand = '0';

void setup() {
    pinMode(Y_LEFT_PUL, OUTPUT);
    pinMode(Y_LEFT_DIR, OUTPUT);
    pinMode(Y_RIGHT_PUL, OUTPUT);
    pinMode(Y_RIGHT_DIR, OUTPUT);

    pinMode(Y_LEFT_SENSOR_BOTTOM, INPUT);
    pinMode(Y_LEFT_SENSOR_TOP, INPUT);
    pinMode(Y_RIGHT_SENSOR_BOTTOM, INPUT);
    pinMode(Y_RIGHT_SENSOR_TOP, INPUT);

    Serial.begin(9600);
}

void loop() {
    if (Serial.available() > 0) {  
        char command = Serial.read(); 

        if (command >= '0' && command <= '9') {
            currentCommand = command;
            motorRunning = (command != '0');
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
            motorRunning = false;
        } 
        else if (currentCommand == '6') {
            Serial.println("Z축 왼쪽 상승 중...");
            digitalWrite(Y_LEFT_DIR, LOW);
            while (digitalRead(Y_LEFT_SENSOR_TOP) == HIGH) {
                generateY_LEFT_PULse();
            }
            Serial.println("top");
            motorRunning = false;
        }
        else if (currentCommand == '7') {
            Serial.println("Z축 오른쪽 하강 중...");
            digitalWrite(Y_RIGHT_DIR, HIGH);
            while (digitalRead(Y_RIGHT_SENSOR_BOTTOM) == HIGH) {
                generateY_RIGHT_PULse();
            }
            Serial.println("bottom");
            motorRunning = false;
        }
        else if (currentCommand == '8') {
            Serial.println("Z축 오른쪽 상승 중...");
            digitalWrite(Y_RIGHT_DIR, LOW);
            while (digitalRead(Y_RIGHT_SENSOR_TOP) == HIGH) {
                generateY_RIGHT_PULse();
            }
            Serial.println("top");
            motorRunning = false;
        }
    } else {
        digitalWrite(Y_LEFT_PUL, LOW);
        digitalWrite(Y_RIGHT_PUL, LOW);
    }
}

// 왼쪽 펄스 생성
void generateY_LEFT_PULse() {
    int period_us = 1000000 / frequency;
    digitalWrite(Y_LEFT_PUL, HIGH);
    delayMicroseconds(period_us / 2);
    digitalWrite(Y_LEFT_PUL, LOW);
    delayMicroseconds(period_us / 2);
}

// 오른쪽 펄스 생성 - 버그 수정됨
void generateY_RIGHT_PULse() {
    int period_us = 1000000 / frequency;
    digitalWrite(Y_RIGHT_PUL, HIGH);
    delayMicroseconds(period_us / 2);
    digitalWrite(Y_RIGHT_PUL, LOW);
    delayMicroseconds(period_us / 2);
}
