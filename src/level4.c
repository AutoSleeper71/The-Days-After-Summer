#include "level4.h"
#include "raylib.h"
#include "game.h"

static int quizMode = 0;

GameState EvaluateEnding()
{
    if (angerBad & depressionBad)
        return ENDING_BAD;

    if (depressionBad)
        return ENDING_SLIGHTLY_BAD;

    return ENDING_GOOD;
}

GameState UpdateLevel4()
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if (!quizMode)
    {
        DrawText("LEVEL 4 - DEPRESSION", w/2 - 220, h/2 - 80, 40, WHITE);

        DrawText("SPACE = negative reaction", w/2 - 200, h/2, 20, WHITE);
        DrawText("G = positive reaction", w/2 - 180, h/2 + 40, 20, WHITE);
        DrawText("T = time quiz", w/2 - 120, h/2 + 80, 20, WHITE);

        if (IsKeyPressed(KEY_SPACE))
        {
            depressionBad++;

            finalEnding = EvaluateEnding();   // ✅ ADD THIS
            nextLevel = finalEnding;

            return ELEVATOR;
        }

        if (IsKeyPressed(KEY_G))
        {
            finalEnding = EvaluateEnding();   // ✅ ADD THIS
            nextLevel = finalEnding;

            return ELEVATOR;
        }

        if (IsKeyPressed(KEY_T))
            quizMode = 1;
    }
    else
    {
        /* quiz later */
    }

    return LEVEL4;
}