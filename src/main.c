/* Main entry point of the game. */

#include "raylib.h"
#include "game.h"
#include "menu.h"
#include "elevator.h"
#include "level1.h"
#include "level2.h"
#include "level3.h"
#include "level4.h"
#include "ending.h"
#include "events.h"

int screenWidth = 1920;
int screenHeight = 1080;

int main(void)
{
    InitWindow(GetMonitorWidth(1920), GetMonitorHeight(1080), "500 Days of Summer");
    SetWindowState(FLAG_FULLSCREEN_MODE);
    SetExitKey(KEY_NULL);

    InitAudioDevice();
    ApplyMasterVolume();

    HideCursor();
    SetTargetFPS(60);

    InitMenu();
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Loading...", 500, 350, 30, WHITE);
    EndDrawing();
    EventsLoadResources();

    GameState state = MENU;

    while (state != GAME_EXIT && !WindowShouldClose())
    {
        GameState newState = state;

        BeginDrawing();
        ClearBackground(BLACK);

        switch(state)
        {
            case MENU: newState = UpdateMenu(); break;
            case ELEVATOR: newState = UpdateElevator(); break;
            case LEVEL1: newState = UpdateLevel1(); break;
            case LEVEL2: newState = UpdateLevel2(); break;
            case LEVEL3: newState = UpdateLevel3(); break;
            case LEVEL4: newState = UpdateLevel4(); break;
            case ENDING_GOOD:
            case ENDING_SLIGHTLY_BAD:
            case ENDING_BAD:
                newState = UpdateEnding(state);
                break;
            case GAME_EXIT:
                newState = GAME_EXIT;
                break;
        }

        EndDrawing();

        if (newState != state)
        {
            CloseSettingsMenu();

            if (newState == MENU)
            {
                InitMenu();
            }
            else if (newState == ELEVATOR)
            {
                InitElevator();
            }
            else if (newState == ENDING_GOOD ||
                     newState == ENDING_SLIGHTLY_BAD ||
                     newState == ENDING_BAD)
            {
                InitEnding(newState);
            }
        }

        state = newState;
    }

    UnloadMenu();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
