#include "game.h"
#include "raylib.h"
#include "elevator.h"

// important uses of some stuff 

// background music

GameState nextLevel = LEVEL1;

// count scores for ending
int angerBad = 0;
int depressionBad = 0;
GameState finalEnding = ENDING_GOOD;

// unlocking the levels

int level1Unlocked = 1;
int level2Unlocked = 0;
int level3Unlocked = 0;
int level4Unlocked = 0;

// resetting the game

void ResetGame()
{
    angerBad = 0;
    depressionBad = 0;
    // weirdAnswer = 0;
    
    nextLevel = LEVEL1;

    InitElevator();
}