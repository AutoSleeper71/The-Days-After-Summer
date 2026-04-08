/* Shared game-wide declarations.
   Other files include this header to access the global state enum and shared counters. */

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

/* High-level game screens.
   The main loop switches on this enum every frame. */
typedef enum
{
    MENU,
    ELEVATOR,
    LEVEL1,
    LEVEL2,
    LEVEL3,
    LEVEL4,
    ENDING_GOOD,
    ENDING_SLIGHTLY_BAD,
    ENDING_BAD,
    GAME_EXIT
} GameState;

/* Result returned by the shared settings overlay. */
typedef enum
{
    SETTINGS_RESULT_NONE,
    SETTINGS_RESULT_BLOCK,
    SETTINGS_RESULT_GO_TO_MENU,
    SETTINGS_RESULT_EXIT
} SettingsResult;

extern GameState nextLevel;
extern GameState finalEnding;

extern Texture2D NONE_TEXTURE;

int Dialog(Texture2D portrait,
           const char *name,
           const char *text,
           const char *opt1,
           const char *opt2,
           const char *opt3);

extern int screenWidth;
extern int screenHeight;

extern int angerBad;
extern int depressionBad;

extern int level1Unlocked;
extern int level2Unlocked;
extern int level3Unlocked;
extern int level4Unlocked;

extern float masterVolume;
void ApplyMasterVolume(void);
void OpenSettingsMenu(void);
void OpenPauseMenu(void);
void CloseSettingsMenu(void);
bool IsSettingsMenuOpen(void);
SettingsResult UpdateAndDrawSettingsMenu(void);

void ResetGame(void);

bool HasSaveGame(void);
void DeleteSaveGame(void);
void SaveGameForState(GameState state);
GameState LoadSavedGame(void);

#endif
