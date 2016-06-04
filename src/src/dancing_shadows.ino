#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 144

CRGB leds[NUM_LEDS];

#define NUM_SPOTS 30

struct spot {
  int shift;
  CRGB color;
  uint8_t x;
  uint8_t width;
  uint8_t type;
};

spot spots[NUM_SPOTS];

void setup() {
  FastLED.addLeds<APA102, MOSI, SCK, BGR>(leds, NUM_LEDS);
  //FastLED.setDither(0);
  for (uint8_t i = 0; i < NUM_SPOTS; i++) {
    spots[i] = random_spot();
  }
}

spot random_spot() {
  spot s;

  s.color = CHSV(random8(), 255, random8());
  s.x = random8(2) * NUM_LEDS;
  s.shift = random8(7) - 3;
  s.width = random8(1, 10);
  s.type = random8(3);

  if (s.shift == 0) {
    s.shift = 1;
  }

  return s;
}

void set_spot(uint16_t center, uint8_t width, uint8_t type, CRGB value) {
  if (width == 0) {
    return;
  } else if (width == 1) {
    leds[center] = blend(leds[center], value, 127);
  } else {
    switch (type) {
      case 0:
      for(uint8_t i = 0; i < width; i++) {
        uint8_t x = min(NUM_LEDS - 1, max(0, (center - width/2) + i));
        leds[x] = blend(leds[x], value, 127);
      }
      break;

      case 1:
      for(uint8_t i = 0; i < width; i++) {
        uint8_t x = min(NUM_LEDS - 1, max(0, (center - width/2) + i));
        leds[x] = blend(leds[x], value - CHSV(0, 0, 255 - dim8_raw(quadwave8(map(i, 0, width - 1, 0, 255)))), 127);
      }
      break;

      case 2:
      for (uint8_t i = 0; i < width; i+=2) {
        uint8_t x = min(NUM_LEDS - 1, max(0, (center - width/2) + i));
        leds[x] = blend(leds[x], value, 127);
      }
      break;

    }
  }
}

uint8_t width = 0;
uint8_t hue = 0;

void loop() {
  FastLED.clear();
  for (uint8_t i = 0; i < NUM_SPOTS; i++) {
    int new_x = spots[i].x + spots[i].shift;

    if (new_x > NUM_LEDS || new_x < 0) {
      spots[i] = random_spot();
    } else {
      spots[i].x = new_x;
    }

    set_spot(spots[i].x, spots[i].width, spots[i].type, spots[i].color);
  }

  FastLED.show();

  delay(50);
}
