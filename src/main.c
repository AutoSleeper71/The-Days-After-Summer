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
    // ToggleFullscreen();
    InitWindow(screenWidth, screenHeight, "500 Days of Summer");
    InitAudioDevice();
    
    HideCursor();
    SetTargetFPS(60);

    InitMenu();
    DrawText("Loading...", 500, 350, 30, WHITE);
    EndDrawing();
    EventsLoadResources();

    GameState state = LEVEL2;

    while (!WindowShouldClose())
    {
        GameState newState = state;

        BeginDrawing();
        ClearBackground(BLACK);

        switch(state)
        {
            case MENU:    newState = UpdateMenu(); break;
            case ELEVATOR:newState = UpdateElevator(); break;
            case LEVEL1:  newState = UpdateLevel1(); break;
            case LEVEL2:  newState = UpdateLevel2(); break;
            case LEVEL3:  newState = UpdateLevel3(); break;
            case LEVEL4:  newState = UpdateLevel4(); break;

            case ENDING_GOOD:
            case ENDING_SLIGHTLY_BAD:
            case ENDING_BAD:
                newState = UpdateEnding(state);
                break;
        }

        EndDrawing();

        if (newState != state)
        {
            if (newState == MENU)
            {
                ResetGame();
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

    UnloadElevator();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}