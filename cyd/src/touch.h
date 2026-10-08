// XPT2046 resistive touch on the ESP32-2432S028R (own pins, bit-banged: HSPI = TFT, VSPI = SD card)
//   T_CLK=25  T_DIN(MOSI)=32  T_DO(MISO)=39  T_CS=33  T_IRQ=36
#pragma once
#include <Arduino.h>

#ifndef TOUCH_CLK
#define TOUCH_CLK  25
#endif
#ifndef TOUCH_MOSI
#define TOUCH_MOSI 32
#endif
#ifndef TOUCH_MISO
#define TOUCH_MISO 39
#endif
#ifndef TOUCH_CS
#define TOUCH_CS   33
#endif
#ifndef TOUCH_IRQ
#define TOUCH_IRQ  36
#endif
// raw calibration (typical CYD values). If taps land in the wrong place, adjust these.
#ifndef TOUCH_X_MIN
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3700
#define TOUCH_Y_MIN 240
#define TOUCH_Y_MAX 3800
#endif

static uint16_t xptXfer(uint8_t cmd) {
  digitalWrite(TOUCH_CS, LOW);
  for (int i = 0; i < 8; i++) {                 // command, MSB first (mode 0)
    digitalWrite(TOUCH_MOSI, (cmd & 0x80) ? HIGH : LOW);
    cmd <<= 1;
    digitalWrite(TOUCH_CLK, HIGH);
    digitalWrite(TOUCH_CLK, LOW);
  }
  uint16_t v = 0;
  for (int i = 0; i < 16; i++) {                // busy bit + 12 data bits + 3 pad
    digitalWrite(TOUCH_CLK, HIGH);
    v = (v << 1) | (digitalRead(TOUCH_MISO) ? 1 : 0);
    digitalWrite(TOUCH_CLK, LOW);
  }
  digitalWrite(TOUCH_CS, HIGH);
  return (v >> 3) & 0x0FFF;
}

static void touchInit() {
  pinMode(TOUCH_CLK, OUTPUT);
  pinMode(TOUCH_MOSI, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  pinMode(TOUCH_MISO, INPUT);
  pinMode(TOUCH_IRQ, INPUT);                    // GPIO36/39 are input-only, no pull-ups (the XPT2046 drives them)
  digitalWrite(TOUCH_CS, HIGH);
  digitalWrite(TOUCH_CLK, LOW);
  xptXfer(0x90);                                // power-down mode with PENIRQ enabled
}

static inline int xptMed3(int a, int b, int c) {
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) { b = a; }
  return b;
}

// returns true while the screen is pressed; x/y = raw 0..4095 (X from cmd 0xD1, Y from cmd 0x91)
static bool touchRaw(int& rx, int& ry) {
#ifdef TOUCH_USE_IRQ
  if (digitalRead(TOUCH_IRQ)) return false;     // IRQ is low while pressed (off by default: pressure polling is more reliable)
#endif
  int z1 = xptXfer(0xB1), z2 = xptXfer(0xC1);
  int z = z1 + 4095 - z2;
  if (z < 400) { xptXfer(0x90); return false; }
  int x0 = xptXfer(0xD1), y0 = xptXfer(0x91);
  int x1 = xptXfer(0xD1), y1 = xptXfer(0x91);
  int x2 = xptXfer(0xD1), y2 = xptXfer(0x91);
  xptXfer(0x90);                                // back to PENIRQ mode
  rx = xptMed3(x0, x1, x2);
  ry = xptMed3(y0, y1, y2);
  return true;
}
