#include <Arduino.h>
#include <LaserWar.h>
#include <LWShoot.h>
#include <LWCommand.h>

LaserWar lw(A0); // Определяем ресивер на пине A0
LWShoot shoot;  // Класс для парсинга выстрелов
LWCommand cmd;  // Класс для парсинга команд

void setup(){
  Serial.begin(9600);
}

void loop() {
  unsigned long s = lw.read(); // Получаем команду через ресивер
  if (s){
    Serial.println(s, HEX); // Выводим полученные данные
    if (cmd.load(s)){
      Serial.println(cmd); // Если полученные данные являются командой, выводим ее
    } else {
      shoot.load(s);
      Serial.println(shoot); // Если полученные данные не являются командой, считаем, что это выстрел
    }
  }
}