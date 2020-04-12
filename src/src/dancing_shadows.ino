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
const int NUM_LEDS =  300;
const int NUM_SPOTS  = 25;
const int BRIGHTNESS = 255;

CRGB leds[NUM_LEDS];

struct spot {
  bool direction_positive;
  uint8_t shift_mod;
  uint8_t shift_counter;
  CRGB color;
  int center;
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

void setup() {
  FastLED.addLeds<APA102, BGR>(leds, NUM_LEDS);

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
  s.center = random8(2) * NUM_LEDS;
  if (s.center == 0) {
    s.center = -random8(100);
  } else {
    s.center += random8(100);
  }

  s.shift_mod = random8(10, 15);
  s.shift_counter = 0;
  s.direction_positive = s.center <= 0;
  s.ticks = 0;

  return s;
}

spot random_spot() {
  spot s;

  s.color = CHSV(random8(), 255, random8(1, BRIGHTNESS));
  s.width = random8(1, 10);
  s.center = random8(2) * (NUM_LEDS + s.width * 2) - s.width;
  s.shift_counter = 0;
  s.shift_mod = (random8(1, 20));
  s.direction_positive = s.center <= 0;
  s.type = random8(4);
  s.ticks = 0;

  return s;
}

spot random_diffuse() {
  spot s;

  s.color = CHSV(random8(), 255, 10);
  s.width = 1;
  s.center = random8(NUM_LEDS - s.width);
  s.shift_counter = 0;
  s.shift_mod = (random8(1, 20));
  s.direction_positive = s.center <= 0;
  s.type = SPOT_TYPE_SOLID;
  s.ticks = 0;

  return s;
}

void blend_led(int x, CRGB color) {
  if (x < 0 || x >= NUM_LEDS) {
    return;
  }

  leds[x] = blend(leds[x], color, 127);
}

void draw_spot(spot &spot) {
  if (spot.width == 0) {
    return;
  }

  if (spot.width == 1) {
    blend_led(spot.center, spot.color);
  } else {
    switch (spot.type) {
      case SPOT_TYPE_SOLID:
      for (int i = 0; i < spot.width; i++) {
        int x = (spot.center - spot.width/2) + i;
        blend_led(x, spot.color);
      }
      break;

      case SPOT_TYPE_GRADIENT:
      for (int i = 0; i < spot.width; i++) {
        int x = (spot.center - spot.width/2) + i;
        blend_led(x, spot.color - CHSV(0, 0,
          255 - dim8_raw(quadwave8(map(i, 0, spot.width - 1, 0, 255)))));
      }
      break;

      case SPOT_TYPE_2X_DOT:
      for (int i = 0; i < spot.width; i+=2) {
        int x = (spot.center - spot.width/2) + i;
        blend_led(x, spot.color);
      }
      break;

      case SPOT_TYPE_3X_DOT:
      for (int i = 0; i < spot.width; i+=3) {
        int x = (spot.center - spot.width/2) + i;
        blend_led(x, spot.color);
      }
      break;
    }
  }
}

boolean advance_spot(spot &s) {
  s.shift_counter = (s.shift_counter + 1) % s.shift_mod;

  if (s.shift_counter == 0) {
    int new_x = s.center + (s.direction_positive ? 1 : -1);

    if ((s.direction_positive && new_x > (NUM_LEDS + s.width))
        || (!s.direction_positive && new_x < -s.width)) {
      return false;
    } else {
      s.center = new_x;
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
          spots[i].color = CHSV(0,
            ((spots[i].center > NUM_LEDS / 2) == spots[i].direction_positive) ? 255 : 0,
             255 - quadwave8((spots[i].center * 255) / NUM_LEDS));
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
    draw_spot(spots[i]);
  }

  FastLED.show();

  delay(1);
}
