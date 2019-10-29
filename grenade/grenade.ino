#include <LaserWar.h>
#include <EEPROM.h>

#define TONE_PIN 0
#define BTN_PIN 1
#define RESET_PIN 2
#define IR_PIN 3
#define BANGS_COUNT 3
#define BEFORE_BANG_DELAY 4
#define RESPAWN_CMD 0x8305e8
#define BANG_CMD 0x830be8
#define SETTINGS_WAIT_TIME 5000

unsigned char used = '0';
unsigned long lastUsedTime = 0;
byte respawnTime = 60;

LaserWar lw;

void saveSettings(){
    EEPROM.write(0, used);
    EEPROM.write(1, respawnTime);
}

void loadSettings(){
    used = EEPROM.read(0);
    if (used != '1' && used != '0'){
        used = '0';
        saveSettings();
    } else {
        respawnTime = EEPROM.read(1);
    }
}

void setup() {
  pinMode(IR_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT);
  loadSettings();
}

void nextMode(){
    if (respawnTime > 180){
        respawnTime = 0;
    } else {
        respawnTime += 60;
    }

    beep(500, 1);
    delay(200);
    beep(200, respawnTime / 60);
}

void settingsMode(){
    beep(200, 3);
    unsigned long keyDownTime = 0;
    while (true){
        unsigned long t = millis();
        if (digitalRead(BTN_PIN) == HIGH){
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

void beep(int duration, byte count){
    for (int i = 0; i < count; i++){
        tone(TONE_PIN, 1000);
        delay(duration);
        noTone(TONE_PIN);
        if (i < count - 1){
            delay(duration);
        }
    }
}

void loop() {
    unsigned long t = millis();
    if (t < SETTINGS_WAIT_TIME){
        if (digitalRead(BTN_PIN) == HIGH){
            settingsMode();
            saveSettings();
            lastUsedTime = 0;
            beep(200, 4);
        }
    } else {  
      if (used == '1'){
          if (respawnTime == 0){
              unsigned long cmd = lw.waitCommand(RESET_PIN);
              if (cmd == RESPAWN_CMD){
                restore();
              }
          } else {
              if (lastUsedTime == 0) lastUsedTime = t;
              if (t - lastUsedTime >= (unsigned long)respawnTime * 1000){
                  restore();
              }
          }
      } else {
          if (digitalRead(BTN_PIN) == HIGH){
            bang();
          }
      }
    }
}

void restore(){
  beep(700, 1);
  used = '0';
  lastUsedTime = 0;
  EEPROM.write(0, used);
}

void bang(){
  beep(500, BEFORE_BANG_DELAY);
  delay(500);

  for (byte i = 0; i < BANGS_COUNT; i++){
    lw.send(IR_PIN, BANG_CMD);
    delay(100);
  }

  used = '1';
  EEPROM.write(0, used);

  beep(2000, 1);
}
