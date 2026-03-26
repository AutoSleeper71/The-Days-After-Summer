#include "level3.h"
#include "raylib.h"
#include "game.h"
#include "zuma.h"

typedef enum {
    LEVEL3_MENU,
    LEVEL3_ZUMA
} Level3State;

static Level3State level3State = LEVEL3_MENU;
static bool zumaInitialized = false;

GameState UpdateLevel3()
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    ClearBackground(BLACK);

    if (level3State == LEVEL3_MENU)
    {
        DrawText("SPACE = never talk to him again", w/2 - 180, h/2, 20, WHITE);
        DrawText("ENTER = it not worth it", w/2 - 150, h/2 + 40, 20, WHITE);
        DrawText("P = help", w/2 - 190, h/2 + 80, 20, WHITE);

        if (IsKeyPressed(KEY_SPACE))
        {
            angerBad += 3;

            nextLevel = LEVEL4;
            level4Unlocked = 1;

            return ELEVATOR;
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            nextLevel = LEVEL4;
            level4Unlocked = 1;

            return ELEVATOR;
        }

        if (IsKeyPressed(KEY_P))
        {
            level3State = LEVEL3_ZUMA;
            zumaInitialized = false; // ✅ ensure fresh init
        }
    }

    else if (level3State == LEVEL3_ZUMA)
    {
        if (!zumaInitialized)
        {
            InitMinigame();
            zumaInitialized = true;
        }

        UpdateMinigame();
        DrawMinigame();

        if (IsMinigameWon())
        {
            DrawText("PRESS ENTER TO FINISH", w/2 - 150, h - 50, 20, YELLOW);

            if (IsKeyPressed(KEY_ENTER))
            {
                int finalScore = GetMinigameScore();

                
                if (finalScore > 100)
                    angerBad += 0;
                else if (finalScore > 50)
                    angerBad += 1;
                else
                    angerBad += 3;

                nextLevel = LEVEL4;
                level4Unlocked = 1;

           
                level3State = LEVEL3_MENU;
                zumaInitialized = false;

                return ELEVATOR;
            }
        }
    }

    return LEVEL3;
}