#include <Arduino.h>
#include <FastLED.h>

/* parameters that could be good for controlling
 *
 * * number of spots
 * * mode
 * * speed
 * * max length of long spot
 * * brightness scale
 * *
 * *
 */
#define NUM_LEDS 144

CRGB leds[NUM_LEDS];

#define NUM_SPOTS 5

struct spot {
  bool direction_positive;
  uint8_t shift_mod;
  uint8_t shift_counter;
  CRGB color;
  int x;
  uint8_t width;
  uint8_t type;
  uint16_t ticks;
};

spot spots[NUM_SPOTS];

#define MODE_RANDOM 0
#define MODE_HEADLIGHTS 1
#define MODE_DIFFUSE 2

#define SPOT_TYPE_SOLID 0
#define SPOT_TYPE_GRADIENT 1
#define SPOT_TYPE_2X_DOT 2
#define SPOT_TYPE_3X_DOT 3

uint8_t mode = MODE_RANDOM;
//uint8_t mode = MODE_HEADLIGHTS;

void setup() {
  FastLED.addLeds<APA102, 4, 5, BGR>(leds, NUM_LEDS);

  for (uint8_t i = 0; i < NUM_SPOTS; i++) {
    switch (mode) {
      case MODE_RANDOM:
        spots[i] = random_spot();
        break;
      case MODE_HEADLIGHTS:
        spots[i] = new_headlights();
        break;
        case MODE_DIFFUSE:
        spots[i] = random_diffuse();
        break;
    }
  }
}


spot new_headlights() {
  spot s;

  s.color = CRGB::White;
  s.type = SPOT_TYPE_3X_DOT;
  s.width = 4;
  s.x = random8(2) * NUM_LEDS;
  if (s.x == 0) {
    s.x = -random8(100);
  } else {
    s.x += random8(100);
  }

  s.shift_mod = random8(10, 15);
  s.shift_counter = 0;
  s.direction_positive = s.x <= 0;
  s.ticks = 0;

  return s;
}

spot random_spot() {
  spot s;

  s.color = CHSV(random8(), 255, random8());
  s.width = random8(1, 10);
  s.x = random8(2) * (NUM_LEDS + s.width * 2) - s.width;
  s.shift_mod = (random8(1, 20));
  s.direction_positive = s.x <= 0;
  s.type = random8(4);
  s.ticks = 0;

  return s;
}

spot random_diffuse() {
  spot s;

  s.color = CHSV(random8(), 255, 10);
  s.width = 1;
  s.x = random8(NUM_LEDS - s.width);
  s.shift_mod = (random8(1, 20));
  s.direction_positive = s.x <= 0;
  s.type = SPOT_TYPE_SOLID;
  s.ticks = 0;

  return s;
}

void blend_led(int x, CRGB color) {
  if (x < 0 || x >= NUM_LEDS) {
    return;
  } else {
    leds[x] = blend(leds[x], color, 127);
  }
}

void draw_spot(uint16_t center, uint8_t width, uint8_t type, CRGB value) {
  if (width == 0) {
    return;
  } else if (width == 1) {
    leds[center] = blend(leds[center], value, 127);
  } else {
    switch (type) {
      case SPOT_TYPE_SOLID:
      for(uint8_t i = 0; i < width; i++) {
        int x = (center - width/2) + i;
        blend_led(x, value);
      }
      break;

      case SPOT_TYPE_GRADIENT:
      for(uint8_t i = 0; i < width; i++) {
        int x = (center - width/2) + i;
        blend_led(x, value - CHSV(0, 0, 255 - dim8_raw(quadwave8(map(i, 0, width - 1, 0, 255)))));
      }
      break;

      case SPOT_TYPE_2X_DOT:
      for (uint8_t i = 0; i < width; i+=2) {
        int x = (center - width/2) + i;
        blend_led(x, value);
      }
      break;

      case SPOT_TYPE_3X_DOT:
      for (uint8_t i = 0; i < width; i+=3) {
        int x = (center - width/2) + i;
        blend_led(x, value);
      }
      break;
    }
  }
}

boolean advance_spot(spot &s) {
  s.shift_counter = (s.shift_counter + 1) % s.shift_mod;

  if (s.shift_counter == 0) {
    int new_x = s.x + (s.direction_positive ? 1 : -1);

    if ((s.direction_positive && new_x > NUM_LEDS + s.width)
    || (!s.direction_positive && new_x < -s.width)) {
      return false;
    } else {
      s.x = new_x;
    }
  }

  return true;
}

void loop() {
  FastLED.clear();
  for (uint8_t i = 0; i < NUM_SPOTS; i++) {
    switch (mode) {
      case MODE_RANDOM:
        if (!advance_spot(spots[i])) {
          spots[i] = random_spot();
        }
        break;
      case MODE_HEADLIGHTS:
        if (!advance_spot(spots[i])) {
          spots[i] = new_headlights();
        } else {
          spots[i].color = CHSV(0, ((spots[i].x > NUM_LEDS / 2) == spots[i].direction_positive) ? 255 : 0, 255 - quadwave8((spots[i].x * 255) / NUM_LEDS));
        }
        break;
      case MODE_DIFFUSE:
        if (spots[i].ticks != 65535) {
          if (spots[i].shift_counter == 0) {
            if (spots[i].ticks < 1024) {
              spots[i].color.maximizeBrightness();
            } else {
              if (spots[i].width < 10) {
                spots[i].width += 1;
              }
              spots[i].color.fadeToBlackBy(10);
              if (spots[i].color.getAverageLight() == 0) {
                spots[i].ticks = 65534;
              }
            }
          }
          spots[i].shift_counter = (spots[i].shift_counter + 1) % spots[i].shift_mod;
        } else {
          spots[i] = random_diffuse();
        }
        break;
    }
    spots[i].ticks += 1;
    draw_spot(spots[i].x, spots[i].width, spots[i].type, spots[i].color);
  }

  FastLED.show();

  delay(1);
}
