#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 12
#define PIN1 8
#define PIN2 9
#define PIN3 10
#define CONTROL_PIN 7  // 메인 아두이노에서 신호 받는 핀

Adafruit_NeoPixel strip1(NUMPIXELS, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUMPIXELS, PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(NUMPIXELS, PIN3, NEO_GRB + NEO_KHZ800);

void setup() {
  strip1.begin(); strip2.begin(); strip3.begin();
  strip1.setBrightness(128);
  strip2.setBrightness(128);
  strip3.setBrightness(128);

  pinMode(CONTROL_PIN, INPUT);
  turnOffAll();
}

void loop() {
  if (digitalRead(CONTROL_PIN) == HIGH) {
    turnOnAll();
  } else {
    turnOffAll();
  }

  delay(50);  // 너무 자주 읽지 않도록
}

void turnOnAll() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip1.setPixelColor(i, strip1.Color(255, 255, 255));
    strip2.setPixelColor(i, strip2.Color(255, 255, 255));
    strip3.setPixelColor(i, strip3.Color(255, 255, 255));
  }
  strip1.show(); strip2.show(); strip3.show();
}

void turnOffAll() {
  strip1.clear(); strip2.clear(); strip3.clear();
  strip1.show(); strip2.show(); strip3.show();
}
