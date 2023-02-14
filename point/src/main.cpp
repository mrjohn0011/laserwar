#include <Arduino.h>
#include <IRremote.h>
#include <TimerMs.h>
#include <EncButton.h>
#include <GRGB.h>

// RGB LED STRIPE with common CATHODE (-) used 
#define RGB_R 5
#define RGB_G 4
#define RGB_B 3
// 12V Active fire alarm is used
#define ALARM_PIN 6
// The button connects 3.3V with PIN. 10K Resistor connects the PIN and GND
#define BUTTON_PIN 7
// An active buzzer 5V is used
#define TONE_PIN 8
// TSOP34856 or TSOP34838 is used
#define TSOP_PIN 9
// Voltage measuring RGB LED
#define VOLTAGE_R 10
#define VOLTAGE_G 11
#define VOLTAGE_B 12
// LaserWar new game command
#define RESPAWN_CMD 0x8305e8
// How many win modes the point has
#define WIN_MODE_COUNT 7
// How many milliseconds after turning on we allow to enter the admin mode by pressing the button
#define SETTINGS_MODE_TIMEOUT 5000
// How many minutes should the team held the point captured to win
unsigned long winModes[WIN_MODE_COUNT] = {2, 4, 6, 8, 10, 20, 30};
// Default win mode after the point turned on (start counting from 0)
unsigned long winMode = 4;

GRGB stripe(COMMON_CATHODE, RGB_R, RGB_G, RGB_B);

/**
 * Voltage measuring
 * Using 1 RGB LED connected to PINS
 * Fully Charged: The voltage is greater than GREEN_VOLTAGE_LEVEL
 * Middle charged: The voltage is greater than BLUE_VOLTAGE_PIN but lower than GREEN_VOLTAGE_LEVEL
 * Discharged and stop working: The voltage is lower than BLUE_VOLTAGE_PIN
 */
#define VOLTAGE_PIN A1
#define GREEN_VOLTAGE_LEVEL 7.8
#define BLUE_VOLTAGE_LEVEL 6.1
GRGB chargeIndicator(COMMON_CATHODE, VOLTAGE_R, VOLTAGE_G, VOLTAGE_B);

double R1 = 1333;  // Battery plus to VOLTAGE_PIN resistor. Can be used any resistor
double R2 = 666;  // VOLTAGE_PIN to GND resistor. Can be used any resistor 2 times smaller than R1
double Vref = 5.0; // Etalon voltage (The voltage on Arduino Ref pin)
double Vmax = Vref*(R1+R2)/R2; // Maximum battery voltage
double k = Vmax/1024; // Measuring koefficient. Always equals Vmax/1023
//=========================================

IRrecv ir(TSOP_PIN);
decode_results results;
const unsigned long RED_COLOR = 0;
const unsigned long BLUE_COLOR = 1;
const unsigned long YELLOW_COLOR = 2;
const unsigned long GREEN_COLOR = 3;
const unsigned long NO_COLOR = 5;
unsigned long activeColor = NO_COLOR;
#define ACTIVE_TIMES_COUNT 6
unsigned long activeTimes[ACTIVE_TIMES_COUNT] = {0, 0, 0, 0, 0, 0};
unsigned long currentTimeStart = 0;

TimerMs winTimer;
TimerMs settingsModeTimer(SETTINGS_MODE_TIMEOUT, true, true);
EncButton<EB_TICK, BUTTON_PIN> btn;

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
    case YELLOW_COLOR:
      Serial.println("YELLOW");
      break;
    default:
      Serial.println("UNDEFINED");
      break;
  }
}

void setRgbColor(unsigned long color){
  switch (color){
  case RED_COLOR:
    stripe.setHEX(0xFF0000);
    break;
  case BLUE_COLOR:
    stripe.setHEX(0x0000FF);
    break;
  case GREEN_COLOR:
    stripe.setHEX(0x00FF00);
  case YELLOW_COLOR:
    stripe.setHEX(0xFFFF00);
    break;
  default:
    stripe.disable();
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
  setRgbColor(NO_COLOR);     
  beep(1000, 1);
  Serial.println("Point reset");
}

void win(unsigned long color){
  Serial.print("WIN COLOR: ");
  showColor(color);
  alarm(1000, 3);
  winTimer.start();
  bool ledOn = false;

  while(true){
    btn.tick();
    if (btn.press()){
      resetPoint();
      return;
    }
    
    if (ir.decode(&results)){
      Serial.print("Signal: "); Serial.println(results.value, HEX);
      if (results.value == RESPAWN_CMD){
        resetPoint();
        winTimer.stop();
        return;
      }
      ir.resume();
      delay(100);
    }

    if (winTimer.tick()){
       setRgbColor(ledOn ? NO_COLOR : color);
       ledOn = !ledOn;
    }
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
  double voltage = analogRead(VOLTAGE_PIN);
  btn.setButtonLevel(HIGH);
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
    Serial.print(winMode);
    Serial.print("; Win minutes = ");
    Serial.println(winModes[winMode]);
    
    beep(500, 1);
    delay(200);
    beep(200, winMode);
}

void settingsMode(){
    Serial.println("Settings mode");
    beep(200, 3);

    while (true){
        btn.tick();
        if (btn.click()){
          nextMode();
        }

        if (btn.held()){
          Serial.println("Exit settings mode");
          beep(200, 4);
          return;
        }
    }
}

void measureVoltage() { 
  double v = analogRead(VOLTAGE_PIN)*k;
  if (v >= GREEN_VOLTAGE_LEVEL){
    chargeIndicator.setHEX(0x00FF00);
  } else if (v >= BLUE_VOLTAGE_LEVEL){
    chargeIndicator.setHEX(0x0000FF);
  } else {
    Serial.println("Low voltage. The point is disabled");
     while (true){
      chargeIndicator.setHEX(0xFF0000);
    }
  }
}

unsigned long currentDuration = 0;

void loop() {
  unsigned long now = millis();
  btn.tick();

  if (settingsModeTimer.active() && !settingsModeTimer.tick() && btn.press()) {
    settingsMode();
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