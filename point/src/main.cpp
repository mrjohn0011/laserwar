#include <Arduino.h>
#include <IRremote.h>

#define RGB_R 10
#define RGB_G 11
#define RGB_B 2

#define ALARM_PIN 6
#define BUTTON_PIN 7
#define TONE_PIN 8
#define TSOP_PIN 9
#define WIN_MODE_COUNT 7
#define RESPAWN_CMD 0x8305e8
unsigned long winModes[WIN_MODE_COUNT] = {2, 4, 6, 8, 10, 20, 30};
unsigned long winMode = 4;
unsigned long settingModeSeconds = 5;

// Voltage measuring
#define VOLTAGE_PIN A1
#define RED_VOLTAGE_PIN 12
#define GREEN_VOLTAGE_PIN 3
#define BLUE_VOLTAGE_PIN 4
double R1 = 1332;  // battery plus to analog pin resistor
double R2 = 666;  // analog pin to ground resistor
double Vref = 4.99; // etalon voltage (ref pin)
double Vmax = Vref*(R1+R2)/R2; // maximum battery voltage
double k = Vmax/1023; // measuring koefficient

IRrecv ir(TSOP_PIN);
decode_results results;
const unsigned long RED_COLOR = 0;
const unsigned long BLUE_COLOR = 1;
const unsigned long GREEN_COLOR = 3;
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
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
    break;
  case BLUE_COLOR:
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
    break;
  case GREEN_COLOR:
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
    break;
  default:
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
    break;
  }
}

void alarm(int duration, int number){
  for (int i = 0; i < number; i++){
    digitalWrite(ALARM_PIN, HIGH);
    delay(duration);
    digitalWrite(ALARM_PIN, LOW);
    delay(duration);
    Serial.println("ALARM");
  }
}

void beep(int duration, int number){
  for (int i = 0; i < number; i++){
    tone(TONE_PIN, 1000);
    delay(duration);
    noTone(TONE_PIN);
    delay(duration);
    Serial.println("BEEP");
  }
}

void resetPoint() {
  activeColor = NO_COLOR; 
  for(int i = 0; i < ACTIVE_TIMES_COUNT; i++){
    activeTimes[i] = 0;
  }
  currentTimeStart = millis();     
  beep(1000, 1);
  Serial.println("Point reset");
}

void win(unsigned long color){
  Serial.print("WIN COLOR: ");
  showColor(color);
  alarm(1000, 3);

  while(true){
    if (digitalRead(BUTTON_PIN) == HIGH){
      resetPoint();
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
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(TSOP_PIN, INPUT);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  analogReference(EXTERNAL);
  double voltage = analogRead(VOLTAGE_PIN);
  ir.enableIRIn();
  Serial.println("Point initialized");
  Serial.print("Maximum battery voltage: ");
  Serial.println(Vmax, 2);
  Serial.print("Actual Voltage: ");
  Serial.print(voltage);
  Serial.print(" = ");
  Serial.print(voltage*k, 2);
  Serial.println("V");
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
        if (digitalRead(BUTTON_PIN) == HIGH){
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

void measureVoltage() {
  double v = analogRead(VOLTAGE_PIN)*k;
  if (v >= 12.0){
    digitalWrite(GREEN_VOLTAGE_PIN, HIGH);
    digitalWrite(BLUE_VOLTAGE_PIN, LOW);
    digitalWrite(RED_VOLTAGE_PIN, LOW);
  } else if (v > 11){
    digitalWrite(GREEN_VOLTAGE_PIN, LOW);
    digitalWrite(BLUE_VOLTAGE_PIN, HIGH);
    digitalWrite(RED_VOLTAGE_PIN, LOW);
  } else {
    digitalWrite(GREEN_VOLTAGE_PIN, LOW);
    digitalWrite(BLUE_VOLTAGE_PIN, LOW);
    digitalWrite(RED_VOLTAGE_PIN, HIGH);
  }
}

unsigned long keyDownTime = 0;
unsigned long currentDuration = 0;

void loop() {
  unsigned long now = millis();

  if (now < settingModeSeconds * 1000){
    if (digitalRead(BUTTON_PIN) == HIGH){
      if (keyDownTime == 0){
          keyDownTime = now;
      } else if (now - keyDownTime >= 1000){
          keyDownTime = 0;
          settingsMode();
          beep(200, 4);
      }
    }
  }

  measureVoltage();

  if (ir.decode(&results)){
    Serial.print("Signal: "); Serial.println(results.value, HEX);
    if (results.value == RESPAWN_CMD){
      resetPoint();
    } else {
      unsigned long color = getColor(results.value);
      if (color != NO_COLOR && color != activeColor){
        currentDuration = (now - currentTimeStart) / 1000;
        activeTimes[activeColor] += currentDuration;
        Serial.print(currentDuration); Serial.print("s for "); showColor(activeColor);
        currentTimeStart = now;
        activeColor = color;
        Serial.print("Color changed to "); showColor(activeColor);
        setRgbColor(activeColor);
        alarm(1000, 1);
      }
    }
    ir.resume();
  }

  if(activeColor != NO_COLOR){
    unsigned long currentDuration = ((now - currentTimeStart) / 1000) + activeTimes[activeColor];
    if(currentDuration > winModes[winMode] * 60){
      Serial.print("Win duration: "); Serial.println(currentDuration);
      Serial.print("Win time: "); Serial.println(winModes[winMode] * 60);
      win(activeColor);
    }
  }

  setRgbColor(activeColor);
}