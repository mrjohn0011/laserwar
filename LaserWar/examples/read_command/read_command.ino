#include <Arduino.h>
#include <LaserWar.h>

/*
Используем фототранзистор 940нм и подключаем с общим эмиттером.
Подключаем короткую ногу (коллектор) через резистор 1 КОм (или 10 КОм) к +5В
Эту же кототкую ногу (коллектор) подключаем к аналоговому пину, например, A0, напрямую
Длинную ногу (эмиттер) подключаем к GND
*/
#define RECEIVER_PIN A0

LaserWar lw(RECEIVER_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop() {
  // Вернет 0, если команда не была считана или команду, если она успешно считалась
  unsigned long cmd = lw.read();
  if (cmd) {
    Serial.println(cmd, HEX);
  }
}