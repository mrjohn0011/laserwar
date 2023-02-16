#include <Arduino.h>
#include <LaserWar.h>

// Use 940nm IR LED to send commands
#define LED_PIN 3
// See all possible commands here https://laserwar.com/support/dataprotocol
#define RADIATION 0xa032e8

LaserWar lw;

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  lw.send(LED_PIN, RADIATION);
  Serial.println(RADIATION);
  delay(5000);
}
