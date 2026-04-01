/* Menu scene API. */

#ifndef MENU_H
#define MENU_H

#include "game.h"

void InitMenu(void); // init function contains images and sounds 
GameState UpdateMenu(void);
void UnloadMenu(void); // unload function to free memory

#endif