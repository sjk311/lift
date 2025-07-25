#include <Adafruit_NeoPixel.h>

#define PIN 8
#define NUMPIXELS 12

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int brightness = 128;  // 초기 밝기 (0~255)

void setup() {
  Serial.begin(9600);
  pixels.begin();
  pixels.setBrightness(brightness);
  pixels.clear();
  pixels.show();

  Serial.println("1: 흰색 ON");
  Serial.println("2: OFF");
  Serial.println("+: 밝기 증가 / -: 밝기 감소");
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();

    if (input == '1') {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 255));
      }
      pixels.show();
    } 
    else if (input == '2') {
      pixels.clear();
      pixels.show();
    }
    else if (input == '+') {
      brightness += 20;
      brightness = constrain(brightness, 0, 255);
      pixels.setBrightness(brightness);
      pixels.show();
      Serial.print("밝기 ↑: ");
      Serial.println(brightness);
    } 
    else if (input == '-') {
      brightness -= 20;
      brightness = constrain(brightness, 0, 255);
      pixels.setBrightness(brightness);
      pixels.show();
      Serial.print("밝기 ↓: ");
      Serial.println(brightness);
    }
  }
}
