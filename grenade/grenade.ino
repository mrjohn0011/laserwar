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

unsigned char used = '0';
LaserWar lw;

void setup() {
  pinMode(IR_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT);
  used = EEPROM.read(0);
  if (used != '1') used = '0';
}

void loop() {
  if (used == '1'){
    unsigned long cmd = lw.waitCommand(RESET_PIN);
    if (cmd == RESPAWN_CMD){
      restore();
    }
  } else {
    if (digitalRead(BTN_PIN) == HIGH){
      bang();
    }
  }
}

void restore(){
  tone(TONE_PIN, 1000);
  delay(700);
  noTone();

  used = '0';
  EEPROM.write(0, used);
}

void bang(){
  for (byte i = 0; i < BEFORE_BANG_DELAY; i++){
    tone(TONE_PIN, 1000);
    delay(500);
    noTone();
    delay(500);
  }

  for (byte i = 0; i < BANGS_COUNT; i++){
    lw.send(IR_PIN, BANG_CMD);
    delay(100);
  }

  used = '1';
  EEPROM.write(0, used);
}
