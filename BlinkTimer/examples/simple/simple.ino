#include <Arduino.h>
#include <BlinkTimer.h>

#define LED_PIN 13

void handler(bool state){
  if (state){
    // What to do each even call of the handler
    Serial.println("NO BEEP");
    digitalWrite(LED_PIN, LOW);
  } else {
    // What to do each odd call of the handler
    Serial.println("BEEP");
    digitalWrite(LED_PIN, HIGH);
  }
}

void onBlinkEnd(){
  // What to do when the task is finished
  Serial.println("Timer Finished");
}

BlinkTimer tmr(handler);

void setup(){
  Serial.begin(9600);
  tmr.setOnFinish(onBlinkEnd);
  /**
   * Blink 3 times with 1000 ms interval. 
   * handler will be called 6 times for each on/off state
   * onBlinkEnd - 1 time when blinking is finished 
   * All blink() calls will be ignored when another blinking is in progress
   * The handler will be executed immediately 1st time and then once per second
   */
  tmr.blink(1000, 3);
}

void loop() {
  tmr.tick(); // Required in the loop method for TimerMs work
}
