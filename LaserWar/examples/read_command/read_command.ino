#include <Arduino.h>
#include <LaserWar.h>

// Use TSOP 34856 to decode signals
#define TSOP_34856_PIN 3

LaserWar lw;

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
  pinMode(TSOP_34856_PIN, INPUT);
}

void loop() {
  unsigned long cmd = lw.waitCommand(TSOP_34856_PIN);
  if (cmd) {
    Serial.println(cmd);
  }
}