#ifndef MINIGAME_H
#define MINIGAME_H

#include "raylib.h"

// when the minigame begin,call this function
void InitMinigame(void);

// in the minigame,update this function every frame loop
void UpdateMinigame(void);

// draw the minigame in the game window
void DrawMinigame(void);

// check if the minigame is finished,used to return to the main game
bool IsMinigameWon(void);

// get the score of the minigame
int GetMinigameScore(void);

#endif