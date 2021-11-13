#include "PointScenario.h"

IRrecv ir(TSOP_PIN);
decode_results results;
Adafruit_NeoPixel pixels(NUMPIXELS, TSOP_PIN, NEO_GRB + NEO_KHZ800);

unsigned long getColor(unsigned long shoot){
  if (shoot < 0x1FFF) {
    unsigned long color = (shoot >> 4) & 0b11;
    return color;
  } else {
    return NO_COLOR;
  }
}

void PointScenario::alarm(int duration, int number){
  for (int i = 0; i < number; i++){
    digitalWrite(ALARM_PIN, HIGH);
    delay(duration);
    digitalWrite(ALARM_PIN, LOW);
    delay(duration);
    Serial.println("ALARM");
  }
}

void PointScenario::beep(int duration, int number){
  for (int i = 0; i < number; i++){
    tone(TONE_PIN, 1000);
    delay(duration);
    noTone(TONE_PIN);
    delay(duration);
    Serial.println("BEEP");
  }
}

void PointScenario::reset(){
    this->setup();
}

void PointScenario::execute(){
    this->setup();

    while (true){
        this->loop();
    }
}

