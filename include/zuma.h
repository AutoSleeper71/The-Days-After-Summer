/* Public interface for the Zuma-style minigame. */

#ifndef MINIGAME_H
#define MINIGAME_H

#include "raylib.h"

// when the minigame begin,call this function
// Start or reset the current minigame round.
void InitMinigame(void);

// in the minigame,update this function every frame loop
// Update gameplay logic once per frame.
void UpdateMinigame(void);

// draw the minigame in the game window
// Draw the minigame once per frame.
void DrawMinigame(void);

// check if the minigame is finished,used to return to the main game
// Check whether all rounds are complete.
bool IsMinigameWon(void);

// get the score of the minigame
int GetMinigameScore(void);

// reset score and stage progress before starting a new session
void ResetMinigameProgress(void);

bool ShouldExitMinigame(void);

#endif