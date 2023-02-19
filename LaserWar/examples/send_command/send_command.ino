#include <Arduino.h>
#include <LaserWar.h>
#include <LWCommand.h>

// Use 940nm IR LED to send commands
#define LED_PIN 3

LaserWar lw(LED_PIN);
// Создаем команду, которую нужно отправить
LWCommand cmd(LwSetting::RadiationDamage, 15);

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  lw.send(LED_PIN, cmd.getCommand()); // Отправляем команду
  Serial.println(cmd);
  delay(5000);
}
