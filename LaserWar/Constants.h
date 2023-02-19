#ifndef LWConstants_h
#define LWConstants_h

#define LW_COLOR_RED 0
#define LW_COLOR_BLUE 1
#define LW_COLOR_YELLOW 2
#define LW_COLOR_GREEN 3
#define LW_CMD_END 0xe8

unsigned const char dmg_list[16] = { 1, 2, 4, 5, 7, 10, 15, 17, 20, 25, 30, 35, 40, 50, 75, 100 };
const char* color_list[4] = {"RED", "BLUE", "YELLOW", "GREEN"};
const char* preset_list[5] = {"ASSAULT", "HOSTAGE", "LAST HERO", "ZOMBIE", "SWAT"};

typedef struct {
    unsigned char code;
    const char* description;
} CommandDescription;

const CommandDescription colorCommands[5] = {
    { 0xA9, "Change color " },
    { 0xB4, "Respawn by color " },
    { 0xB5, "Pause by color " },
    { 0xB6, "Kill by color " },
    { 0xB7, "Full ammo by color " }
};

const CommandDescription dataCommands[11] = {
    { 0x80, "Add health " },
    { 0x81, "Add ammo " },
    { 0x8A, "Add magazines " },
    { 0x8B, "Medic kit. Add health " },
    { 0xA0, "Radiation damage " },
    { 0xA1, "Anomaly. Add health " },
    { 0xAA, "Play sound " },
    { 0xB8, "Respawn by ID " },
    { 0xB9, "Pause by ID " },
    { 0xBA, "Kill by ID " },
    { 0xBB, "Full ammo by ID " }
};

const CommandDescription commands83[26] = {
    { 0x00, "Kill" },
    { 0x01, "Pause/Resume" },
    { 0x02, "Start game" },
    { 0x03, "Default settings" },
    { 0x04, "Respawn" },
    { 0x05, "New game" },
    { 0x06, "Full ammo" },
    { 0x07, "End game" },
    { 0x08, "Clear time" },
    { 0x09, "Change color Red <> Blue, Green/Yellow -> Red" },
    { 0x0A, "Init player" },
    { 0x0B, "Explode player" },
    { 0x0C, "New game" },
    { 0x0D, "Full health" },
    { 0x0E, "Capasity: <50% -> 99%, >50% -> 49%" },
    { 0x0F, "Full ammo" },
    { 0x10, "Double health until death" },
    { 0x11, "Checkpoint captured" },
    { 0x12, "Bomb deactivated" },
    { 0x14, "Clear statistics" },
    { 0x15, "Handband test" },
    { 0xE8, "Deactivate player" },
    { 0x17, "Remove weapon" },
    { 0x20, "Increase damage until death" },
    { 0x21, "Set 750 shoots per minute" },
    { 0x22, "Double health once" }
};

#endif