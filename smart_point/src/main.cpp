#include <BasePointScenario.h>

BasePointScenario s;

void setup(){
    Serial.begin(9600);
}

void loop(){
    s.execute();
}