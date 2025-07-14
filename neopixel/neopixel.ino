#include <Adafruit_NeoPixel.h>

#define PIN        8        // 데이터 입력 핀
#define NUMPIXELS  12       // LED 개수

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();           // NeoPixel 초기화
  pixels.setBrightness(255); // 밝기 최대 (0~255)
}

void loop() {
  showColor(255, 0, 0);     // 빨강
  delay(100);

  showColor(0, 0, 255);     // 파랑
  delay(100);

  showColor(255, 255, 0);   // 노랑 (빨강 + 초록)
  delay(100);

  showColor(0, 255, 0);     // 초록
  delay(100);
}

void showColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
