#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 12

#define PIN1 8
#define PIN2 9
#define PIN3 10

Adafruit_NeoPixel strip1(NUMPIXELS, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS, PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(NUMPIXELS, PIN3, NEO_GRB + NEO_KHZ800);

int brightness = 128;  // 초기 밝기 (0~255)

void setup() {
  Serial.begin(9600);

  strip1.begin();
  strip2.begin();
  strip3.begin();

  strip1.setBrightness(brightness);
  strip2.setBrightness(brightness);
  strip3.setBrightness(brightness);

  turnOnWhite();  // 전원 연결 시 흰색 점등

  Serial.println("1: 흰색 ON");
  Serial.println("2: OFF");
  Serial.println("+: 밝기 증가 / -: 밝기 감소");
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();

    if (input == '1') {
      turnOnWhite();
    } 
    else if (input == '2') {
      turnOff();
    }
    else if (input == '+') {
      brightness += 20;
      brightness = constrain(brightness, 0, 255);
      updateBrightness();
      Serial.print("밝기 ↑: ");
      Serial.println(brightness);
    } 
    else if (input == '-') {
      brightness -= 20;
      brightness = constrain(brightness, 0, 255);
      updateBrightness();
      Serial.print("밝기 ↓: ");
      Serial.println(brightness);
    }
  }
}

void turnOnWhite() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip1.setPixelColor(i, strip1.Color(255, 255, 255));
    strip2.setPixelColor(i, strip2.Color(255, 255, 255));
    strip3.setPixelColor(i, strip3.Color(255, 255, 255));
  }
  strip1.show();
  strip2.show();
  strip3.show();
}

void turnOff() {
  strip1.clear();
  strip2.clear();
  strip3.clear();
  strip1.show();
  strip2.show();
  strip3.show();
}

void updateBrightness() {
  strip1.setBrightness(brightness);
  strip2.setBrightness(brightness);
  strip3.setBrightness(brightness);
  // 밝기 변경 후 현재 색상 유지 위해 show() 호출
  strip1.show();
  strip2.show();
  strip3.show();
}
