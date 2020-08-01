#include <Arduino.h>
#include <IRremote.h>

#define LIGHT_TIME 3
#define TSOP_PIN 2

#define COLOR_RED 0
#define COLOR_BLUE 1
#define COLOR_YELLOW 2
#define COLOR_GREEN 4

#define R_PIN 9
#define G_PIN 10
#define B_PIN 11
#define LIGHT_TIME 3

IRrecv ir(TSOP_PIN);
decode_results results;

unsigned long getColor(unsigned long shoot){
    if (shoot >= 0x1FFF) return -1;
    return (shoot & 0x30) >> 4;
}

void blinkColor(int r, int g, int b){
    analogWrite(R_PIN, r);
    analogWrite(G_PIN, g);
    analogWrite(B_PIN, b);
    delay(LIGHT_TIME * 1000);
}

void setup(){
    Serial.begin(9600);
    Serial.println("Enabling IR");
    ir.enableIRIn();
    Serial.println("Enabled IR");
    Serial.println();
}

void loop(){
    if (ir.decode(&results)){
        Serial.println(results.value, HEX);
        byte color = getColor(results.value);

        switch (color) {
            case COLOR_RED:
                Serial.println("RED");
                blinkColor(255, 0, 0);
                break;
            case COLOR_BLUE:
                Serial.println("BLUE");
                blinkColor(0, 0, 255);
                break;
            case COLOR_GREEN:
                Serial.println("GREEN");
                blinkColor(0, 255, 0);
                break;
            case COLOR_YELLOW:
                Serial.println("YELLOW");
                blinkColor(255, 255, 0);
                break;
            default:
                Serial.println("Color is not detected");
                break;
        }

        ir.resume();
    }
    analogWrite(R_PIN, 0);
    analogWrite(G_PIN, 0);
    analogWrite(B_PIN, 0);
    delay(100);
}
