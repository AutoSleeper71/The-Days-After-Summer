/* Shared game-wide state.
   This file stores ending counters, progression flags, and the reset logic. */

#include "game.h"
#include "raylib.h"
#include "elevator.h"

// important uses of some stuff 

// The elevator reads this value to know which playable scene should come next.
GameState nextLevel = LEVEL1;

// count scores for ending
// These counters are increased by player choices/minigames and later used to choose an ending.
int angerBad = 0;
int depressionBad = 0;
GameState finalEnding = ENDING_GOOD;

// unlocking the levels

int level1Unlocked = 1;
int level2Unlocked = 0;
int level3Unlocked = 0;
int level4Unlocked = 0;

// resetting the game

/* Reset shared run data so a fresh playthrough starts from a clean state. */
void ResetGame()
{
    angerBad = 0;
    depressionBad = 0;
    // weirdAnswer = 0;
    
    nextLevel = LEVEL1;

    InitElevator();
}