#include <Arduino.h>
#include <IRremote.h>
#define rgb_r 3
#define rgb_g 10
#define rgb_b 11
#define tsop 9
#define tonePin 8
#define button 7
#define WIN_MODE_COUNT 7
unsigned long winModes[WIN_MODE_COUNT] = {2, 4, 6, 8, 10, 20, 30};
unsigned long winMode = 0;
unsigned long settingModeSeconds = 5;

IRrecv ir(tsop);
decode_results results;
const unsigned long RED_COLOR = 0;
const unsigned long BLUE_COLOR = 1;
const unsigned long GREEN_COLOR = 4;
const unsigned long NO_COLOR = 5;
unsigned long activeColor = NO_COLOR;
#define ACTIVE_TIMES_COUNT 6
unsigned long activeTimes[ACTIVE_TIMES_COUNT] = {0, 0, 0, 0, 0, 0};
unsigned long currentTimeStart = 0;

void showColor(unsigned long color){
  switch (color) {
    case RED_COLOR:
      Serial.println("RED");
      break;
    case BLUE_COLOR:
      Serial.println("BLUE");
      break;
    case GREEN_COLOR:
      Serial.println("GREEN");
      break;
    default:
      Serial.println("UNDEFINED");
      break;
  }
}

void setRgbColor(unsigned long color){
  switch (color){
  case RED_COLOR:
    digitalWrite(rgb_r, HIGH);
    digitalWrite(rgb_g, LOW);
    digitalWrite(rgb_b, LOW);
    break;
  case BLUE_COLOR:
    digitalWrite(rgb_r, LOW);
    digitalWrite(rgb_g, LOW);
    digitalWrite(rgb_b, HIGH);
    break;
  case GREEN_COLOR:
    digitalWrite(rgb_r, LOW);
    digitalWrite(rgb_g, HIGH);
    digitalWrite(rgb_b, LOW);
    break;
  default:
    digitalWrite(rgb_r, LOW);
    digitalWrite(rgb_g, LOW);
    digitalWrite(rgb_b, LOW);
    break;
  }
}

void beep(int duration, int number){
  for (int i = 0; i < number; i++){
    tone(tonePin, 1000);
    delay(duration);
    noTone(tonePin);
    delay(duration);
    Serial.println("BEEP");
  }
}

void win(unsigned long color){
  Serial.print("WIN COLOR: ");
  showColor(color);

  for(int i = 0; i < ACTIVE_TIMES_COUNT; i++){
    activeTimes[i] = 0;
  }
  currentTimeStart = millis();
  activeColor = NO_COLOR;
  beep(1000, 3);

  while(true){
    if (digitalRead(button) == HIGH){
      beep(1000, 1);
      Serial.println("Point reset");
      return;
    }
    
    setRgbColor(color);
    delay(1000);
    setRgbColor(NO_COLOR);
    delay(1000);
  }
}

unsigned long getColor(unsigned long shoot){
  if (shoot < 0x1FFF) {
    unsigned long color = (shoot >> 4) & 0b11;
    return color;
  } else {
    return NO_COLOR;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(rgb_r, OUTPUT);
  pinMode(rgb_g, OUTPUT);
  pinMode(rgb_b, OUTPUT);
  pinMode(tsop, INPUT);
  pinMode(tonePin, OUTPUT);
  pinMode(button, INPUT);
  ir.enableIRIn();
  Serial.println("Point initialized");
}

void nextMode(){
    if (winMode == WIN_MODE_COUNT - 1){
        winMode = 0;
    } else {
        winMode++;
    }

    Serial.print("Win Mode = ");
    Serial.println(winMode);
    
    beep(500, 1);
    delay(200);
    beep(200, winMode);
    
    Serial.print("Win minutes = ");
    Serial.println(winModes[winMode]);
}

void settingsMode(){
    Serial.println("Settings mode");
    beep(200, 3);
    unsigned long keyDownTime = 0;
    while (true){
        unsigned long t = millis();
        if (digitalRead(button) == HIGH){
            if (keyDownTime == 0){
                keyDownTime = t;
            } else if (t - keyDownTime > 1000){
                keyDownTime = 0;
                return;
            }
        } else if (keyDownTime > 0) {
            if (keyDownTime > 0 && t - keyDownTime < 1000){
                keyDownTime = 0;
                nextMode();
            }
        }
    }
}

unsigned long keyDownTime = 0;
unsigned long currentDuration = 0;
void loop() {
  unsigned long now = millis();

  if (now < settingModeSeconds * 1000){
    if (digitalRead(button) == HIGH){
      if (keyDownTime == 0){
          keyDownTime = now;
      } else if (now - keyDownTime >= 1000){
          keyDownTime = 0;
          settingsMode();
          beep(200, 4);
      }
    }
  }

  if (ir.decode(&results)){
    Serial.print("Signal: "); Serial.println(results.value, HEX);
    unsigned long color = getColor(results.value);
    if (color != NO_COLOR && color != activeColor){
      currentDuration = (now - currentTimeStart) / 1000;
      activeTimes[activeColor] += currentDuration;
      Serial.print(currentDuration); Serial.print("s for "); showColor(activeColor);
      currentTimeStart = now;
      activeColor = color;
      Serial.print("Color changed to "); showColor(activeColor);
      setRgbColor(activeColor);
      beep(1000, 1);
    }
    ir.resume();
  }

  if(activeColor != NO_COLOR){
    unsigned long currentDuration = ((now - currentTimeStart) / 1000) + activeTimes[activeColor];
    if(currentDuration > winModes[winMode] * 60){
      Serial.print("Win duration: "); Serial.println(currentDuration);
      Serial.print("Linit: "); Serial.println(winModes[winMode] * 60);
      win(activeColor);
    }
  }

  setRgbColor(activeColor);
}