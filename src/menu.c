/* Title screen and main menu.
   Handles menu input, title music, and launching a new or saved run. */

#include "menu.h"
#include "raylib.h"
#include "events.h"
#include "game.h"

int selected = 0;
float titleScreen_timer = 0.0f;
int show_menu = 0;

Texture2D titleScreen;
Sound moveSound;
Music backgroundMusic;

void InitMenu(void)
{
    selected = 0;
    titleScreen_timer = 0.0f;
    show_menu = 0;

    if (!titleScreen.id) titleScreen = LoadTexture("assets/titleScreen.png");
    if (!moveSound.frameCount) moveSound = LoadSound("assets/choose_option.mp3");
    if (!backgroundMusic.ctxData) backgroundMusic = LoadMusicStream("assets/title_screen_song.mp3");

    SetMusicVolume(backgroundMusic, 1.0f);
    ApplyMasterVolume();

    if (!IsMusicStreamPlaying(backgroundMusic))
        PlayMusicStream(backgroundMusic);

    CloseSettingsMenu();
}

GameState UpdateMenu(void)
{
    bool canContinue = HasSaveGame();
    Color continueColor = canContinue ? RAYWHITE : GRAY;

    DrawTextureEx(titleScreen, (Vector2){0,0}, 0.0f, 1.0f, WHITE);

    titleScreen_timer += GetFrameTime();
    UpdateMusicStream(backgroundMusic);

    if (titleScreen_timer > 0.0f)
        show_menu = 1;

    DrawText("THE DAYS AFTER SUMMER", 120, 180, 80, RAYWHITE);

    if (show_menu)
    {
        if (!IsSettingsMenuOpen())
        {
            if (IsKeyPressed(KEY_DOWN))
            {
                selected++;
                SetSoundVolume(moveSound, 0.8f);
                PlaySound(moveSound);
            }
            if (IsKeyPressed(KEY_UP))
            {
                selected--;
                SetSoundVolume(moveSound, 0.8f);
                PlaySound(moveSound);
            }

            if (selected < 0) selected = 3;
            if (selected > 3) selected = 0;
        }

        DrawText(selected == 0 ? "> START"    : "  START",    140, 300, 50, RAYWHITE);
        DrawText(selected == 1 ? "> CONTINUE" : "  CONTINUE", 140, 370, 50, continueColor);
        DrawText(selected == 2 ? "> SETTINGS" : "  SETTINGS", 140, 440, 50, RAYWHITE);
        DrawText(selected == 3 ? "> EXIT"     : "  EXIT",     140, 510, 50, RAYWHITE);

        if (!IsSettingsMenuOpen() && IsKeyPressed(KEY_ENTER))
        {
            if (selected == 0)
            {
                DeleteSaveGame();
                ResetGame();
                StopMusicStream(backgroundMusic);
                return ELEVATOR;
            }

            if (selected == 1 && canContinue)
            {
                GameState loaded = LoadSavedGame();
                if (loaded != MENU)
                {
                    StopMusicStream(backgroundMusic);
                    return loaded;
                }
            }

            if (selected == 2)
                OpenSettingsMenu();

            if (selected == 3)
                return GAME_EXIT;
        }
    }

    {
        SettingsResult settings = UpdateAndDrawSettingsMenu();
        if (settings == SETTINGS_RESULT_EXIT)
            return GAME_EXIT;
    }

    return MENU;
}

void UnloadMenu(void)
{
    if (titleScreen.id) UnloadTexture(titleScreen);
    if (moveSound.frameCount) UnloadSound(moveSound);
    if (backgroundMusic.ctxData) UnloadMusicStream(backgroundMusic);

    titleScreen = (Texture2D){0};
    moveSound = (Sound){0};
    backgroundMusic = (Music){0};
}
