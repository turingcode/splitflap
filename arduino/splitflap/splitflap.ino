/*
   Copyright 2015-2016 Scott Bezek and the splitflap contributors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <avr/power.h>
#include <math.h>
#include <SPI.h>
#include "splitflap_module.h"

#define NEOPIXEL_DEBUGGING_ENABLED false

#if NEOPIXEL_DEBUGGING_ENABLED
#include <Adafruit_NeoPixel.h>
#endif

const uint8_t flaps[] = {
  ' ',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '.',
  ',',
  '\'',
};

#define DEBUG_LED_0_DDR DDRC
#define DEBUG_LED_0_PORT PORTC
#define DEBUG_LED_0_BIT 0

#define DEBUG_LED_0_PIN (14)
#define DEBUG_LED_1_PIN (10)
#define OUT_LATCH_PIN (7)
#define IN_LATCH_PIN (8)

#define OUT_LATCH_PORT PORTD
#define OUT_LATCH_BIT 7
#define IN_LATCH_PORT PORTB
#define IN_LATCH_BIT 0


#define NUM_MODULES (12)
#define MOTOR_BUFFER_LENGTH (NUM_MODULES / 2 + (NUM_MODULES % 2 != 0))
uint8_t motor_buffer[MOTOR_BUFFER_LENGTH];

#define SENSOR_BUFFER_LENGTH (NUM_MODULES / 4 + (NUM_MODULES % 4 != 0))
uint8_t sensor_buffer[SENSOR_BUFFER_LENGTH];

SplitflapModule moduleA(motor_buffer[0], 0, sensor_buffer[0], B00000001);
SplitflapModule moduleB(motor_buffer[0], 4, sensor_buffer[0], B00000010);
SplitflapModule moduleC(motor_buffer[1], 0, sensor_buffer[0], B00000100);
SplitflapModule moduleD(motor_buffer[1], 4, sensor_buffer[0], B00001000);

#if NUM_MODULES > 4
SplitflapModule moduleE(motor_buffer[2], 0, sensor_buffer[1], B00000001);
SplitflapModule moduleF(motor_buffer[2], 4, sensor_buffer[1], B00000010);
SplitflapModule moduleG(motor_buffer[3], 0, sensor_buffer[1], B00000100);
SplitflapModule moduleH(motor_buffer[3], 4, sensor_buffer[1], B00001000);
#endif

#if NUM_MODULES > 8
SplitflapModule moduleI(motor_buffer[4], 0, sensor_buffer[2], B00000001);
SplitflapModule moduleJ(motor_buffer[4], 4, sensor_buffer[2], B00000010);
SplitflapModule moduleK(motor_buffer[5], 0, sensor_buffer[2], B00000100);
SplitflapModule moduleL(motor_buffer[5], 4, sensor_buffer[2], B00001000);
#endif

SplitflapModule modules[] = {
  moduleA,
  moduleB,
  moduleC,
  moduleD,

#if NUM_MODULES > 4
  moduleE,
  moduleF,
  moduleG,
  moduleH,
#endif

#if NUM_MODULES > 8
  moduleI,
  moduleJ,
  moduleK,
  moduleL,
#endif
};
int recv_buffer[NUM_MODULES];

#if NEOPIXEL_DEBUGGING_ENABLED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_MODULES, 6, NEO_GRB + NEO_KHZ800);
uint32_t color_green = strip.Color(0, 50, 0);
uint32_t color_red = strip.Color(100, 0, 0);
uint32_t color_teal = strip.Color(0, 50, 50);
uint32_t color_orange = strip.Color(50, 20, 0);
#endif

inline void spi_transfer() {
  IN_LATCH_PORT &= ~(1 << IN_LATCH_BIT);
  IN_LATCH_PORT |= (1 << IN_LATCH_BIT);

  for (uint8_t i = 0; i < MOTOR_BUFFER_LENGTH; i++) {
    int val = SPI.transfer(motor_buffer[MOTOR_BUFFER_LENGTH - 1 - i]);
    if (i < SENSOR_BUFFER_LENGTH) {
      sensor_buffer[i] = val;
    }
  }

  OUT_LATCH_PORT |= (1 << OUT_LATCH_BIT);
  OUT_LATCH_PORT &= ~(1 << OUT_LATCH_BIT);
}

void setup() {
  Serial.begin(115200);

  pinMode(DEBUG_LED_0_PIN, OUTPUT);
  pinMode(DEBUG_LED_1_PIN, OUTPUT);

  for (uint8_t i = 0; i < MOTOR_BUFFER_LENGTH; i++) {
    motor_buffer[i] = 0;
  }
  for (uint8_t i = 0; i < SENSOR_BUFFER_LENGTH; i++) {
    sensor_buffer[i] = 0;
  }

  // Initialize SPI
  pinMode(IN_LATCH_PIN, OUTPUT);
  pinMode(OUT_LATCH_PIN, OUTPUT);
  digitalWrite(IN_LATCH_PIN, HIGH);
  SPI.begin();
  SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));
  spi_transfer();

  Serial.print("Starting.\nNum modules: ");
  Serial.print(NUM_MODULES);
  Serial.print("\nMotor buffer length: ");
  Serial.print(MOTOR_BUFFER_LENGTH);
  Serial.print("\nSensor buffer length: ");
  Serial.print(SENSOR_BUFFER_LENGTH);
  Serial.print("\n");

#if NEOPIXEL_DEBUGGING_ENABLED
  strip.begin();
  strip.show();
#endif

  // Pulse DEBUG_LED_1_PIN for fun
  digitalWrite(DEBUG_LED_0_PIN, HIGH);
  for (int16_t i = 0; i < 200; i++) {
    analogWrite(DEBUG_LED_1_PIN, i);
#if NEOPIXEL_DEBUGGING_ENABLED
    for (int j = 0; j < NUM_MODULES; j++) {
      strip.setPixelColor(j, i, 0, 0);
    }
    strip.show();
#endif
    delay(3);
  }

  for (int16_t i = 200; i >= 0; i--) {
    analogWrite(DEBUG_LED_1_PIN, i);
#if NEOPIXEL_DEBUGGING_ENABLED
    for (int j = 0; j < NUM_MODULES; j++) {
      strip.setPixelColor(j, i, 0, 0);
    }
    strip.show();
#endif
    delay(3);
  }

  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    recv_buffer[i] = 0;
    modules[i].Init();
    modules[i].GoHome();
  }
  digitalWrite(DEBUG_LED_0_PIN, LOW);
}


inline int8_t FindFlapIndex(uint8_t character) {
    for (int8_t i = 0; i < NUM_FLAPS; i++) {
        if (character == flaps[i]) {
          return i;
        }
    }
    return -1;
}


bool was_idle = false;
uint8_t recv_count = 0;
void loop() {
  while (1) {
    boolean all_idle = true;
    boolean any_bad_timing = false;
    DEBUG_LED_0_PORT |= (1 << DEBUG_LED_0_BIT);
    for (uint8_t i = 0; i < NUM_MODULES; i++) {
      any_bad_timing |= modules[i].Update();
      bool is_idle = modules[i].state == PANIC
#if HOME_CALIBRATION_ENABLED
        || modules[i].state == LOOK_FOR_HOME
        || modules[i].state == SENSOR_ERROR
#endif
        || (modules[i].state == NORMAL && modules[i].current_accel_step == 0);
      all_idle &= is_idle;
    }
    DEBUG_LED_0_PORT &= ~(1 << DEBUG_LED_0_BIT);
    spi_transfer();

    if (all_idle) {

#if NEOPIXEL_DEBUGGING_ENABLED
      for (int i = 0; i < NUM_MODULES; i++) {
        uint32_t color;
        switch (modules[i].state) {
          case NORMAL:
            color = color_green;
            break;
          case LOOK_FOR_HOME:
            color = color_teal;
            break;
          case SENSOR_ERROR:
            color = color_orange;
            break;
          case PANIC:
            color = color_red;
            break;
        }
        strip.setPixelColor(i, color);
      }
      strip.show();
#endif

      if (Serial.available() > 0) {
        int b = Serial.read();
        switch (b) {
          case '@':
            for (uint8_t i = 0; i < NUM_MODULES; i++) {
              modules[i].GoHome();
            }
            break;
          case '=':
            recv_count = 0;
            break;
          case '\n':
              Serial.print("Going to '");
              for (uint8_t i = 0; i < NUM_MODULES; i++) {
                int8_t index = FindFlapIndex(recv_buffer[i]);
                if (index != -1) {
                  modules[i].GoToFlapIndex(index);
                }
                Serial.write(recv_buffer[i]);
              }
              Serial.print("'\n");
              break;
          default:
            if (recv_count > NUM_MODULES - 1) {
              break;
            }
            recv_buffer[recv_count] = b;
            recv_count++;
            break;
        }
      }
  
      if (!was_idle) {
        for (uint8_t i = 0; i < NUM_MODULES; i++) {
          Serial.print("---\nStats ");
          Serial.print(i);
          Serial.print(":\nM: ");
          Serial.print(modules[i].count_missed_home);
          Serial.print("\nU: ");
          Serial.print(modules[i].count_unexpected_home);
          Serial.print("\n");
        }
        Serial.print("##########\n");
      }
    }
    was_idle = all_idle;
  }
}
