#include <Arduino.h>
#include <IRremote.h>
#include <EEPROM.h>

// 3-bit rgb codes
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define PURPLE 5
#define YELLOW 6
#define WHITE 7 

// your pinout may be different, please check your board 
#define TSOP_PIN 8
#define R_PIN 7
#define G_PIN 10
#define B_PIN 11

#define EEPROM_COLORSET_ADDR 0
#define EEPROM_DAMAGE_COOLDOWN_ADDR 1

#define NEW_GAME_COMMAND 0x8305E8
#define KILL_COMMAND 0x8300E8

struct ColorSet {
    const byte size;
    const byte* colors;

    ColorSet(byte size, byte* colors): size(size), colors(colors) { }
};

const ColorSet* const colorSets[] = { 
    new ColorSet(3, (byte[]){ RED, PURPLE, WHITE }),
    new ColorSet(1, (byte[]){ WHITE }),
    new ColorSet(1, (byte[]){ RED }),
    new ColorSet(1, (byte[]){ GREEN }),
    new ColorSet(1, (byte[]){ BLUE }),
    new ColorSet(3, (byte[]){ GREEN, RED, BLUE }),
    new ColorSet(3, (byte[]){ BLUE, RED, GREEN }),
    new ColorSet(3, (byte[]){ BLUE, GREEN, WHITE }),
    new ColorSet(3, (byte[]){ BLUE, RED, WHITE }) 
};
const byte colorSetCount = sizeof(colorSets) / sizeof(colorSets[0]);
byte colorSetIndex = 0;

const unsigned int damageCooldowns[] { 50, 200, 500, 1000 };
const byte damageCooldownCount = sizeof(damageCooldowns) / sizeof(damageCooldowns[0]);
byte damageCooldownIndex = 3;

unsigned int damage = 0;
unsigned long lastHitTime = 0;

unsigned int getDamageCooldown() {
    return damageCooldowns[damageCooldownIndex];
}

const ColorSet* getColorSet() {
    return colorSets[colorSetIndex];
}

void setColor(byte color) {
    analogWrite(R_PIN, (color & 4) ? 255 : 0);
    analogWrite(G_PIN, (color & 2) ? 255 : 0);
    analogWrite(B_PIN, (color & 1) ? 255 : 0);
}

void setDamage(unsigned int newDamage) {
    if (newDamage == damage) {
        return;
    }
    
    damage = newDamage;
    const ColorSet* colors = getColorSet();
    byte color = damage == 0
        ? BLACK
        : colors->colors[min(damage, colors->size) - 1];
    setColor(color);
}

void handleShot() {
    lastHitTime = millis();
    setDamage(damage + 1);
}

void previewColors() {
    const ColorSet* colors = getColorSet();
    for (byte c = 0; c < colors->size; c++) {
        setColor(colors->colors[c]);
        delay(getDamageCooldown());
    }
    setColor(BLACK);
}

void setNextColorSet() {
    setDamage(0);
    colorSetIndex = (colorSetIndex + 1) % colorSetCount;
    EEPROM.update(EEPROM_COLORSET_ADDR, colorSetIndex);
    previewColors();
}

void setNextDamageCooldown() {
    setDamage(0);
    damageCooldownIndex = (damageCooldownIndex + 1) % damageCooldownCount;
    EEPROM.update(EEPROM_DAMAGE_COOLDOWN_ADDR, damageCooldownIndex);
    previewColors();
}

void handleCommand(unsigned long command) {
    switch (command) {
        case NEW_GAME_COMMAND:
            setNextColorSet();
            break;
        case KILL_COMMAND:
            setNextDamageCooldown();
            break;
        default:
            handleShot();
    }
}

IRrecv ir(TSOP_PIN);

void setup() {
    // Serial.begin(9600);
    ir.enableIRIn();
    colorSetIndex = EEPROM.read(EEPROM_COLORSET_ADDR) % colorSetCount;
    damageCooldownIndex = EEPROM.read(EEPROM_DAMAGE_COOLDOWN_ADDR) % damageCooldownCount;
    previewColors();
}

void loop() {
    if (millis() - lastHitTime > getDamageCooldown()) {
        setDamage(0);
    }

    decode_results results;
    if (ir.decode(&results)) {
        if (results.bits == 24) {
            handleCommand(results.value);
        } else if (results.bits == 14) {
            handleShot();
        }
        ir.resume();
    }

    delay(10);
}