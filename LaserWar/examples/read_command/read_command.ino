#include <LaserWar.h>
#define TSOP_PIN 3

LaserWar lw;

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
  pinMode(TSOP_PIN, INPUT);
}

void loop() {
  unsigned long cmd = lw.waitCommand(TSOP_PIN);
  if (cmd != 0) {
    Serial.println(cmd);
  }
}