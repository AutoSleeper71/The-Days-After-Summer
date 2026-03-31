/* Shared game-wide declarations.
   Other files include this header to access the global state enum and shared counters. */

#ifndef GAME_H
#define GAME_H

#include "raylib.h"



// states of the game, to know where we are and where to go next

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
    ENDING_BAD
} GameState;

// Scene scripts write to this before returning to the elevator so the elevator knows where to go next.
extern GameState nextLevel;

// Cached ending value chosen at the end of the run.
extern GameState finalEnding;

// dialog system

extern Texture2D NONE_TEXTURE;

int Dialog(Texture2D portrait,
           const char *name,
           const char *text,
           const char *opt1,
           const char *opt2,
           const char *opt3);

// needed to not write one same code again, but ill fix it later
extern int screenWidth;
extern int screenHeight;

// ending system
// Ending counters collected across levels.
extern int angerBad;
extern int depressionBad;
// extern int weirdAnswer;

// unlocking levels system

extern int level1Unlocked;
extern int level2Unlocked;
extern int level3Unlocked;
extern int level4Unlocked;

// reset
void ResetGame();

#endif