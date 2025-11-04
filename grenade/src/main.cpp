#include <Arduino.h>
#include <EEPROMex.h>
#include <LaserWar.h>
#include <LWCommand.h>
#include <EncButton.h>
#include <TimerMs.h>

#define TONE_PIN 0
#define BTN_PIN 3
#define TSOP_PIN 2
#define IR_PIN 1
#define BANGS_COUNT 3
#define BEFORE_BANG_DELAY 4
#define SETTINGS_WAIT_TIME 5000
#define MINUTES_BEFORE_SEARCH 30

unsigned char used = '0';
#define RESPAWN_MODES_COUNT 5
unsigned long respawnTimes[RESPAWN_MODES_COUNT] = {0, 5000, 60000, 300000, 600000};
byte respawnMode = 2;

LaserWar lwSender(IR_PIN);
LaserWar lwReceiver(TSOP_PIN);
LWCommand bangCmd(LwSetting::AdminCommand, LwAdminSetting::Explode);
LWCommand respawnCmd;

EncButton<EB_TICK, BTN_PIN> btn;
TimerMs settingsTimer(SETTINGS_WAIT_TIME, true, true);
TimerMs searchTimer(MINUTES_BEFORE_SEARCH * 60000, false, true);
TimerMs respawnTimer(respawnTimes[respawnMode], false, true);

void beep(int duration, byte count)
{
    Serial.print("BEEP: ");
    Serial.print(duration);
    Serial.print("; ");
    Serial.println(count);
    for (int i = 0; i < count; i++)
    {
        tone(TONE_PIN, 1000);
        delay(duration);
        noTone(TONE_PIN);
        if (i < count - 1)
        {
            delay(duration);
        }
    }
    Serial.println("BEEP END");
}

void restore()
{
    beep(700, 1);
    used = '0';
    respawnTimer.stop();
    searchTimer.start();
    Serial.println("Restore!");
    EEPROM.write(0, used);
}

void bang()
{
    beep(500, BEFORE_BANG_DELAY);
    delay(500);

    for (byte i = 0; i < BANGS_COUNT; i++)
    {
        lwSender.send(bangCmd);
        delay(100);
    }
    Serial.println("Bang!");

    used = '1';
    searchTimer.start();
    if (respawnMode != 0)
    {
        respawnTimer.start();
    }
    EEPROM.write(0, used);

    beep(2000, 1);
}

void saveSettings()
{
    EEPROM.write(0, used);
    EEPROM.write(1, respawnMode);
    Serial.print("Saved settings: ");
    Serial.print("Respawn Mode = ");
    Serial.print(respawnMode);
    respawnTimer.setTime(respawnTimes[respawnMode]);
    respawnTimer.start();
    Serial.print("; Used = ");
    Serial.println(used == '0' ? 0 : 1);
}

void loadSettings()
{
    used = EEPROM.read(0);
    if (used != '1' && used != '0')
    {
        used = '0';
        saveSettings();
    }
    else
    {
        respawnMode = EEPROM.read(1);
        if (respawnMode != 0)
        {
            respawnTimer.setTime(respawnTimes[respawnMode]);
            respawnTimer.start();
        }
    }
    Serial.print("Loaded settings: ");
    Serial.print("Respawn Mode = ");
    Serial.print(respawnMode);
    Serial.print("; Used = ");
    Serial.println(used == '0' ? 0 : 1);
}

void setup()
{
    pinMode(IR_PIN, OUTPUT);
    pinMode(TSOP_PIN, INPUT);
    pinMode(BTN_PIN, INPUT);
    pinMode(TONE_PIN, OUTPUT);
    btn.setButtonLevel(LOW);
    Serial.begin(9600);
    loadSettings();
    beep(100, 1);
}

void nextMode()
{
    if (respawnMode == RESPAWN_MODES_COUNT - 1)
    {
        respawnMode = 0;
    }
    else
    {
        respawnMode++;
    }

    Serial.print("Respawn Mode = ");
    Serial.println(respawnMode);

    beep(500, 1);
    delay(200);
    beep(200, respawnMode);

    Serial.print("Respawn Time = ");
    Serial.println(respawnTimes[respawnMode]);
}

void settingsMode()
{
    Serial.println("Settings mode");
    beep(200, 3);
    while (true)
    {
        btn.tick();
        if (btn.click())
        {
            nextMode();
        }
        if (btn.held())
        {
            saveSettings();
            beep(200, 4);
            return;
        }
    }
}

void searchMode()
{
    searchTimer.setTime(200);
    searchTimer.setPeriodMode();
    searchTimer.start();
    bool state = true;

    while (true)
    {
        btn.tick();
        if (searchTimer.tick())
        {
            state ? tone(TONE_PIN, 1000) : noTone(TONE_PIN);
            state = !state;
        }

        if (btn.click())
        {
            searchTimer.setTime(MINUTES_BEFORE_SEARCH * 60000);
            searchTimer.setTimerMode();
            searchTimer.start();
            beep(100, 1);
            return;
        }
    }
}

void loop()
{
    btn.tick();
    settingsTimer.tick();

    if (settingsTimer.active() && btn.press())
    {
        settingsMode();
    }
    if (searchTimer.tick())
    {
        searchMode();
        delay(2000);
    }

    if (used == '0')
    {
        if (btn.click())
        {
            bang();
        }
    }
    else
    {
        if (respawnMode == 0)
        {
            unsigned long cmd = lwReceiver.read();
            if (respawnCmd.load(cmd))
            {
                if (respawnCmd.getCommand() == LWCommand(LwSetting::AdminCommand, LwAdminSetting::NewGame).getCommand())
                {
                    restore();
                }
            }
        }
        else
        {
            if (respawnTimer.tick())
            {
                restore();
            }
        }
    }
}
