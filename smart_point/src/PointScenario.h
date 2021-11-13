#ifndef PointScenario_h
#define PointScenario_h

#include "Arduino.h"
#include "IRremote.h"
#include "Adafruit_NeoPixel.h"

#define ALARM_PIN 6
#define TONE_PIN 8
#define TSOP_PIN 9
#define NUMPIXELS 16

#define RED_COLOR 0
#define BLUE_COLOR 1
#define GREEN_COLOR 3
#define NO_COLOR 5

#define RESPAWN_CMD 0x8305e8

class PointScenario {
    public:
        void execute();
    protected:
        void alarm(int duration, int number);
        void beep(int duration, int number);
        void win(unsigned long color);
        unsigned long getColor(unsigned long shoot);

        /**
         * Resets all variables to start the game with the scenario from the very beginning
         * calls setup method by default 
        */
        virtual void reset();

        /**
         * Executes once before the scenario started
        */
        virtual void setup() = 0;

        /**
         * Executes in the infinite loop once the scenario started
        */
        virtual void loop() = 0;
};

#endif