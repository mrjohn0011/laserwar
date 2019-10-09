#include <EEPROM.h>
#include <LaserWar.h>

#define LED_PIN 13
#define TONE_PIN 4
#define DAMAGE_PIN 3
#define INTERVAL_PIN 2

#define DAMAGE_STEP 5
#define INTERVAL_STEP 5

#define MIN_DAMAGE 5
#define MAX_DAMAGE 100

#define MIN_INTERVAL 5
#define MAX_INTERVAL 120

#define LONG_PRESS_DURATION 500

byte damage = MIN_DAMAGE;
byte interval = MIN_INTERVAL;
bool settingsMode = false;

unsigned long RADIATION_CMD = 0xA005E8;
LaserWar lw;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);
  
  Serial.println("Loading settings: ");
  damage = EEPROM.read(0);
  interval = EEPROM.read(1);
  Serial.print("damage: "); Serial.println(damage);
  Serial.print("interval: "); Serial.println(interval);
  
  if (!inRange(damage, MIN_DAMAGE, MAX_DAMAGE)) {
    damage = MIN_DAMAGE;
    Serial.print("Damage is incorrect. Set min damage: "); Serial.println(damage);
  }

  if (!inRange(interval, MIN_INTERVAL, MAX_INTERVAL)) {
    interval = MIN_INTERVAL;
    Serial.print("Interval is incorrect. Set min interval: "); Serial.println(interval);
  }

  createRadiationCmd();
}

bool inRange(byte value, byte min, byte max){
  return value >= min && value <= max;
}

byte fitIntoRange(byte value, byte min, byte max){
  if (value > max) return value - max;
  if (value < min) return max - (min - value);
  return value;
}

void beep(int t){
  Serial.print("Beep: "); Serial.println(t);
  tone(TONE_PIN, 1000);
  delay(t);
  noTone(TONE_PIN);
}

void save(){
  EEPROM.write(0, damage);
  Serial.print("Save new damage: "); Serial.println(damage);

  EEPROM.write(1, interval);
  Serial.print("Save new interval: "); Serial.println(interval);
  createRadiationCmd();

  beep(200); delay(200);
  beep(200); delay(200);
  beep(200);
}

void createRadiationCmd(){
  RADIATION_CMD = 0xa000e8 | ((unsigned long)damage << 8);
}

void clean(){
  damage = MIN_DAMAGE;
  interval = MIN_INTERVAL;

  save();
}

void damageIncrease(){
  damage = fitIntoRange(damage + DAMAGE_STEP, MIN_DAMAGE, MAX_DAMAGE);
  Serial.print("Increase damage: "); Serial.println(damage);
  beep(400);
}

void intervalIncrease(){
  interval = fitIntoRange(interval + INTERVAL_STEP, MIN_INTERVAL, MAX_INTERVAL);
  Serial.print("Increase interval: "); Serial.println(interval);
  beep(400);
}

void settings(){
  unsigned long dmgKeyDownTime = 0;
  unsigned long intKeyDownTime = 0;
  Serial.println("Settings mode");
  beep(200); delay(200); beep(200); delay(200);
  
  while(true){
    unsigned long t = millis();
    
    if (digitalRead(DAMAGE_PIN) == HIGH){
      if (!dmgKeyDownTime) {
        dmgKeyDownTime = t;
      } else {
        if (t - dmgKeyDownTime > LONG_PRESS_DURATION){
          save();
          dmgKeyDownTime = 0;
          break;
        }
      }
    } else {
      if (dmgKeyDownTime && t - dmgKeyDownTime < LONG_PRESS_DURATION){
        damageIncrease();
        dmgKeyDownTime = 0;
      }
    }

   if (digitalRead(INTERVAL_PIN) == HIGH){
      if (!intKeyDownTime) {
        intKeyDownTime = t;
      } else {
        if (t - intKeyDownTime > LONG_PRESS_DURATION){
          clean();
          delay(200);
          beep(200);
          intKeyDownTime = 0;
          break;
        }
      }
    } else {
      if (intKeyDownTime && t - intKeyDownTime < LONG_PRESS_DURATION){
        intervalIncrease();
        intKeyDownTime = 0;
      }
    } 
  }
}

unsigned long lastRadiationTime = 0;
void loop(){ 
  unsigned long t = millis();

  if (digitalRead(DAMAGE_PIN) == HIGH){
    settings();
    delay(500);
  }

  if (t - lastRadiationTime >= interval * 1000){
    lw.send(LED_PIN, RADIATION_CMD);
    Serial.print("RADIATION: ");
    Serial.println(RADIATION_CMD);
    lastRadiationTime = t;
  }
}
