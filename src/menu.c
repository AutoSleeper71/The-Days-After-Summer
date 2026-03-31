/* Title screen and main menu.
   Handles menu input, music, and the first transition into gameplay. */

#include "menu.h"
#include "raylib.h"
#include "events.h"

int selected = 0;

float titleScreen_timer = 0.0;
int show_menu = 0;

Texture2D titleScreen;
Sound moveSound;
Music backgroundMusic;

// initialization function, since otherwise it just loads every frame
/* Load menu assets and reset menu-specific state.
   This is called once when entering the menu scene. */
void InitMenu()
{
	selected = 0;
    titleScreen_timer = 0.0;
    show_menu = 0;
    titleScreen = LoadTexture("assets/titleScreen.png");
    moveSound = LoadSound("assets/choose_option.mp3");
	backgroundMusic = LoadMusicStream("assets/title_screen_song.mp3");

	SetMusicVolume(backgroundMusic, 0.2f);
    PlayMusicStream(backgroundMusic);

}

/* Draw and update the menu every frame.
   Returns the next game state when the player makes a selection. */
GameState UpdateMenu()
{	
	DrawTextureEx(titleScreen, (Vector2){0,0}, 0.0f, 1.0f, WHITE);

	titleScreen_timer += GetFrameTime();

	UpdateMusicStream(backgroundMusic);
	
	// few seconds to enjoy title screen
	if (titleScreen_timer > 0.0f) {
	    show_menu = 1;
	}

	// main text
	DrawText("THE DAYS AFTER SUMMER", 120, 180, 80, RAYWHITE);
	
	if (show_menu) {
	// keyboard navigation
	    if (IsKeyPressed(KEY_DOWN)) {
	    	selected++;
			SetSoundVolume(moveSound, 0.8f);
	    	PlaySound(moveSound);
		}
	    if (IsKeyPressed(KEY_UP)) {
	    	selected--;
			SetSoundVolume(moveSound, 0.8f);
	    	PlaySound(moveSound);
		}
	
	    if (selected < 0) selected = 2;
	    if (selected > 2) selected = 0;

    // menu options
	    if (selected == 0)
		    DrawText("> START", 140, 300, 50, RAYWHITE);
		else
		    DrawText("  START", 140, 300, 50, RAYWHITE);
		
		
		if (selected == 1)
		    DrawText("> CONTINUE", 140, 370, 50, RAYWHITE);
		else
		    DrawText("  CONTINUE", 140, 370, 50, RAYWHITE);
		
		
		if (selected == 2)
		    DrawText("> EXIT", 140, 440, 50, RAYWHITE);
		else
		    DrawText("  EXIT", 140, 440, 50, RAYWHITE);
	
	    // selection
	    if (IsKeyPressed(KEY_ENTER))
	    {
	        if (selected == 0)
	            return ELEVATOR; 
	
	        if (selected == 1)
	        {
	            // CONTINUE not implemented yet
	        }
	
	        if (selected == 2)
	            CloseWindow();
	    }
	}
    
    return MENU;
}

/* Free menu-only assets.
   Useful if you later decide to unload the menu scene cleanly. */
void UnloadMenu()
{
    UnloadTexture(titleScreen);
    UnloadSound(moveSound);
}