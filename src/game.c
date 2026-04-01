/* Shared game-wide state.
   Stores progression, settings, and save/load helpers. */

#include "game.h"
#include "raylib.h"
#include "elevator.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
    int version;
    int state;
    int nextLevelState;
    int finalEndingState;
    int angerBad;
    int depressionBad;
    int level1Unlocked;
    int level2Unlocked;
    int level3Unlocked;
    int level4Unlocked;
    float masterVolume;
} SaveData;

#define SAVEGAME_VERSION 1
#define SAVEGAME_PATH "savegame.dat"

GameState nextLevel = LEVEL1;
int angerBad = 0;
int depressionBad = 0;
GameState finalEnding = ENDING_GOOD;

int level1Unlocked = 1;
int level2Unlocked = 0;
int level3Unlocked = 0;
int level4Unlocked = 0;

float masterVolume = 0.20f;

static bool settingsOpen = false;
static bool settingsFromPause = false;
static int settingsSelected = 0;

static bool IsValidSavedState(int state)
{
    return state >= ELEVATOR && state <= ENDING_BAD;
}

void ApplyMasterVolume(void)
{
    if (masterVolume < 0.0f) masterVolume = 0.0f;
    if (masterVolume > 1.0f) masterVolume = 1.0f;
    SetMasterVolume(masterVolume);
}

void OpenSettingsMenu(void)
{
    settingsOpen = true;
    settingsFromPause = false;
    settingsSelected = 0;
}

void OpenPauseMenu(void)
{
    settingsOpen = true;
    settingsFromPause = true;
    settingsSelected = 0;
}

void CloseSettingsMenu(void)
{
    settingsOpen = false;
    settingsFromPause = false;
}

bool IsSettingsMenuOpen(void)
{
    return settingsOpen;
}

static void DrawSettingsSlider(float x, float y, float width, float value, bool selected)
{
    float fill = width * value;
    DrawRectangle((int)x, (int)y, (int)width, 12, Fade(LIGHTGRAY, 0.35f));
    DrawRectangle((int)x, (int)y, (int)fill, 12, WHITE);
    DrawRectangleLines((int)x, (int)y, (int)width, 12, selected ? YELLOW : GRAY);

    float knobX = x + fill - 6.0f;
    if (knobX < x - 2.0f) knobX = x - 2.0f;
    if (knobX > x + width - 10.0f) knobX = x + width - 10.0f;
    DrawRectangle((int)knobX, (int)y - 4, 12, 20, selected ? YELLOW : RAYWHITE);
}

SettingsResult UpdateAndDrawSettingsMenu(void)
{
    if (!settingsOpen) return SETTINGS_RESULT_NONE;

    int lastIndex = settingsFromPause ? 3 : 2;

    if (IsKeyPressed(KEY_ESCAPE))
    {
        CloseSettingsMenu();
        return SETTINGS_RESULT_BLOCK;
    }

    if (IsKeyPressed(KEY_UP)) settingsSelected--;
    if (IsKeyPressed(KEY_DOWN)) settingsSelected++;

    if (settingsSelected < 0) settingsSelected = lastIndex;
    if (settingsSelected > lastIndex) settingsSelected = 0;

    if (settingsSelected == 0)
    {
        if (IsKeyPressed(KEY_LEFT))
        {
            masterVolume -= 0.05f;
            ApplyMasterVolume();
        }
        if (IsKeyPressed(KEY_RIGHT))
        {
            masterVolume += 0.05f;
            ApplyMasterVolume();
        }
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        if (settingsSelected == 1)
        {
            CloseSettingsMenu();
            return SETTINGS_RESULT_BLOCK;
        }
        if (settingsFromPause && settingsSelected == 2)
        {
            CloseSettingsMenu();
            return SETTINGS_RESULT_GO_TO_MENU;
        }
        if ((settingsFromPause && settingsSelected == 3) || (!settingsFromPause && settingsSelected == 2))
        {
            return SETTINGS_RESULT_EXIT;
        }
    }

    int w = GetScreenWidth();
    int h = GetScreenHeight();
    Rectangle panel = { w/2.0f - 360.0f, h/2.0f - 250.0f, 720.0f, 500.0f };

    DrawRectangle(0, 0, w, h, Fade(BLACK, 0.60f));
    DrawRectangleRounded(panel, 0.08f, 10, Fade((Color){18, 18, 26, 245}, 0.98f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 10, 2.0f, RAYWHITE);

    DrawText("SETTINGS", (int)panel.x + 245, (int)panel.y + 28, 42, RAYWHITE);

    Color soundColor = (settingsSelected == 0) ? YELLOW : RAYWHITE;
    Color backColor  = (settingsSelected == 1) ? YELLOW : RAYWHITE;
    Color menuColor  = (settingsSelected == 2 && settingsFromPause) ? YELLOW : RAYWHITE;
    Color exitColor  = ((settingsSelected == 3 && settingsFromPause) || (settingsSelected == 2 && !settingsFromPause)) ? YELLOW : RAYWHITE;

    DrawText("SOUND", (int)panel.x + 50, (int)panel.y + 105, 30, soundColor);
    DrawSettingsSlider(panel.x + 210.0f, panel.y + 118.0f, 300.0f, masterVolume, settingsSelected == 0);
    DrawText(TextFormat("%d%%", (int)(masterVolume * 100.0f + 0.5f)), (int)panel.x + 535, (int)panel.y + 105, 30, soundColor);

    DrawText(settingsSelected == 1 ? "> BACK" : "  BACK", (int)panel.x + 50, (int)panel.y + 185, 30, backColor);

    if (settingsFromPause)
    {
        DrawText(settingsSelected == 2 ? "> GO TO MENU" : "  GO TO MENU", (int)panel.x + 50, (int)panel.y + 235, 30, menuColor);
        DrawText(settingsSelected == 3 ? "> EXIT GAME" : "  EXIT GAME", (int)panel.x + 50, (int)panel.y + 285, 30, exitColor);
    }
    else
    {
        DrawText(settingsSelected == 2 ? "> EXIT GAME" : "  EXIT GAME", (int)panel.x + 50, (int)panel.y + 235, 30, exitColor);
    }

    DrawText("INGAME CONTROLS", (int)panel.x + 50, (int)panel.y + 325, 28, RAYWHITE);
    DrawText("ENTER = skip / continue dialogue", (int)panel.x + 50, (int)panel.y + 365, 24, LIGHTGRAY);
    DrawText("ESC = open or close settings", (int)panel.x + 50, (int)panel.y + 397, 24, LIGHTGRAY);
    DrawText("A / G / H = battle controls in level 3", (int)panel.x + 50, (int)panel.y + 429, 24, LIGHTGRAY);
    DrawText("MOUSE LEFT CLICK = shoot in Zuma minigame", (int)panel.x + 50, (int)panel.y + 461, 24, LIGHTGRAY);

    return SETTINGS_RESULT_BLOCK;
}

void ResetGame(void)
{
    angerBad = 0;
    depressionBad = 0;
    nextLevel = LEVEL1;
    finalEnding = ENDING_GOOD;

    level1Unlocked = 1;
    level2Unlocked = 0;
    level3Unlocked = 0;
    level4Unlocked = 0;

    CloseSettingsMenu();
    InitElevator();
}

bool HasSaveGame(void)
{
    FILE *fp = fopen(SAVEGAME_PATH, "rb");
    if (!fp) return false;
    fclose(fp);
    return true;
}

void DeleteSaveGame(void)
{
    remove(SAVEGAME_PATH);
}

void SaveGameForState(GameState state)
{
    SaveData data;
    FILE *fp;

    data.version = SAVEGAME_VERSION;
    data.state = (int)state;
    data.nextLevelState = (int)nextLevel;
    data.finalEndingState = (int)finalEnding;
    data.angerBad = angerBad;
    data.depressionBad = depressionBad;
    data.level1Unlocked = level1Unlocked;
    data.level2Unlocked = level2Unlocked;
    data.level3Unlocked = level3Unlocked;
    data.level4Unlocked = level4Unlocked;
    data.masterVolume = masterVolume;

    fp = fopen(SAVEGAME_PATH, "wb");
    if (!fp) return;
    fwrite(&data, sizeof(data), 1, fp);
    fclose(fp);
}

GameState LoadSavedGame(void)
{
    SaveData data;
    FILE *fp = fopen(SAVEGAME_PATH, "rb");

    if (!fp)
        return MENU;

    if (fread(&data, sizeof(data), 1, fp) != 1)
    {
        fclose(fp);
        return MENU;
    }

    fclose(fp);

    if (data.version != SAVEGAME_VERSION || !IsValidSavedState(data.state))
        return MENU;

    nextLevel = IsValidSavedState(data.nextLevelState) ? (GameState)data.nextLevelState : LEVEL1;
    finalEnding = IsValidSavedState(data.finalEndingState) ? (GameState)data.finalEndingState : ENDING_GOOD;
    angerBad = data.angerBad;
    depressionBad = data.depressionBad;
    level1Unlocked = data.level1Unlocked;
    level2Unlocked = data.level2Unlocked;
    level3Unlocked = data.level3Unlocked;
    level4Unlocked = data.level4Unlocked;
    masterVolume = data.masterVolume;
    ApplyMasterVolume();
    CloseSettingsMenu();

    return (GameState)data.state;
}
