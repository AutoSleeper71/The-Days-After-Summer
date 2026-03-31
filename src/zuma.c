/* Zuma-style letter minigame.
   The player fires letters into a moving chain to complete answers and clear each round. */

#include "zuma.h"
#include "raymath.h"
#include <string.h>
#include <math.h>

// File-specific compile-time limit used to size arrays safely.
#define MAX_LETTERS 100
#define MAX_PROJECTILES 20
#define LETTER_RADIUS 15
#define BULLET_SPEED 10.0f
#define LETTER_SPACING 35.0f

extern int depressionBad;

/* One moving letter in the chain.
   distance stores how far along the path that letter has traveled. */
typedef struct {
    char charVal;
    float distance;
    Color color;
} Letter;

/* One fired projectile from the player. */
typedef struct {
    char charVal;
    Vector2 pos;
    Vector2 velocity;
    bool active;
} Projectile;

/* Question/answer pair for one minigame round. */
typedef struct {
    const char* question;
    const char* answer;
} MiniLevelData;

static MiniLevelData miniLevels[] = {
    {"1 + 1 = ?", "TWO"},
    {"CAPITAL OF FRANCE?", "PARIS"},
    {"OPPOSITE OF HOT?", "COLD"}
};

static int miniTotalLevels = 3;
static int miniCurrentLevel = 0;
static int miniScore = 0;

static Letter miniChain[MAX_LETTERS];
static int miniChainCount = 0;

static Projectile miniProjectiles[MAX_PROJECTILES];

static int answerIndex = 0;
static char miniNextBulletChar;

static bool miniGameFinished = false;

////////////////////////////////////////////////////////////
// ПУТЬ 
////////////////////////////////////////////////////////////
/* Convert a normalized progress value into a screen position on the U-shaped path. */
static Vector2 GetPathPoint(float t) {
    float w = GetScreenWidth();
    float h = GetScreenHeight();

    float margin = 80;

    float left = margin;
    float right = w - margin;
    float top = margin;
    float bottom = h - margin;

    if (t < 0.33f) {
        float k = t / 0.33f;
        return (Vector2){ left, top + k * (bottom - top) };
    }
    else if (t < 0.66f) {
        float k = (t - 0.33f) / 0.33f;
        return (Vector2){ left + k * (right - left), bottom };
    }
    else {
        float k = (t - 0.66f) / 0.34f;
        return (Vector2){ right, bottom - k * (bottom - top) };
    }
}

////////////////////////////////////////////////////////////

/* Scan the moving chain for the full answer string.
   When the answer appears, remove it and either advance to the next round or finish the minigame. */
static void CheckMatch(void) {
    const char* ans = miniLevels[miniCurrentLevel].answer;
    int len = strlen(ans);

    for (int i = 0; i <= miniChainCount - len; i++) {
        bool match = true;

        for (int j = 0; j < len; j++) {
            if (miniChain[i + j].charVal != ans[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            for (int k = i + len; k < miniChainCount; k++)
                miniChain[k - len] = miniChain[k];

            miniChainCount -= len;
            miniScore++;

            if (miniChainCount <= 0) {
                miniCurrentLevel++;

                if (miniCurrentLevel >= miniTotalLevels)
                    miniGameFinished = true;
                else
                    InitMinigame();
            }
            return;
        }
    }
}

////////////////////////////////////////////////////////////

/* Reset the current round and build the starting chain from the visible question text. */
void InitMinigame(void) {
    if (miniCurrentLevel >= miniTotalLevels) {
        miniGameFinished = true;
        return;
    }

    miniChainCount = 0;
    miniGameFinished = false;

    for (int i = 0; i < MAX_PROJECTILES; i++)
        miniProjectiles[i].active = false;

    const char* q = miniLevels[miniCurrentLevel].question;

    for (int i = 0; i < strlen(q); i++) {
        if (q[i] != ' ') {
            miniChain[miniChainCount].charVal = q[i];
            miniChain[miniChainCount].distance = -miniChainCount * LETTER_SPACING;
            miniChain[miniChainCount].color = RED;
            miniChainCount++;
        }
    }

    answerIndex = 0;
    miniNextBulletChar = miniLevels[miniCurrentLevel].answer[0];
}

////////////////////////////////////////////////////////////

/* Advance chain movement, fire projectiles, handle collisions, and check win/loss conditions. */
void UpdateMinigame(void) {

    if (miniGameFinished) return;

    float speed = 0.4f + miniCurrentLevel * 0.6f;

    for (int i = 0; i < miniChainCount; i++)
        miniChain[i].distance += speed;

    // ПРОИГРЫШ
    if (miniChainCount > 0) {
        float lastT = miniChain[miniChainCount - 1].distance / 1000.0f;

        // If the tail of the chain reaches the end of the path, count it as a failure for this round.
        if (lastT >= 1.0f) {
            depressionBad++;

            miniCurrentLevel++;

            if (miniCurrentLevel >= miniTotalLevels)
                miniGameFinished = true;
            else
                InitMinigame();

            return;
        }
    }

    Vector2 center = {
        GetScreenWidth()/2.0f,
        GetScreenHeight()/2.0f
    };

    // Clicking fires the next required answer letter from the center of the screen toward the mouse cursor.
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (!miniProjectiles[i].active) {

                miniProjectiles[i].active = true;
                miniProjectiles[i].pos = center;
                miniProjectiles[i].charVal = miniNextBulletChar;

                Vector2 dir = Vector2Normalize(
                    Vector2Subtract(GetMousePosition(), center));

                miniProjectiles[i].velocity =
                    Vector2Scale(dir, BULLET_SPEED);

                const char* ans = miniLevels[miniCurrentLevel].answer;
                int len = strlen(ans);

                answerIndex++;
                if (answerIndex >= len) answerIndex = 0;

                miniNextBulletChar = ans[answerIndex];
                break;
            }
        }
    }

    // trajectory update
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!miniProjectiles[i].active) continue;

        miniProjectiles[i].pos =
            Vector2Add(miniProjectiles[i].pos, miniProjectiles[i].velocity);

        int bestIndex = -1;
        float bestDist = 999999.0f;

        for (int j = 0; j < miniChainCount; j++) {
            Vector2 lp = GetPathPoint(miniChain[j].distance / 1000.0f);
            float d = Vector2Distance(miniProjectiles[i].pos, lp);

            if (d < bestDist) {
                bestDist = d;
                bestIndex = j;
            }
        }

        // Close-enough projectiles snap into the chain, then the game checks whether the answer word was formed.
        if (bestDist < LETTER_RADIUS * 3.5f && bestIndex != -1) {

            int j = bestIndex;
            Vector2 lp = GetPathPoint(miniChain[j].distance / 1000.0f);

            // магнит
            miniProjectiles[i].pos = lp;

            for (int k = miniChainCount; k > j; k--)
                miniChain[k] = miniChain[k - 1];

            miniChain[j].charVal = miniProjectiles[i].charVal;

            if (j < miniChainCount - 1)
                miniChain[j].distance =
                    miniChain[j + 1].distance - LETTER_SPACING;
            else
                miniChain[j].distance =
                    miniChain[j - 1].distance + LETTER_SPACING;

            miniChain[j].color = BLUE;
            miniChainCount++;

            miniProjectiles[i].active = false;

            CheckMatch();
        }

        if (miniProjectiles[i].pos.x < 0 ||
            miniProjectiles[i].pos.x > GetScreenWidth() ||
            miniProjectiles[i].pos.y < 0 ||
            miniProjectiles[i].pos.y > GetScreenHeight())
            miniProjectiles[i].active = false;
    }
}

////////////////////////////////////////////////////////////

/* Render the aiming guide, path, moving chain, projectiles, and HUD text. */
void DrawMinigame(void) {

    Vector2 center = {
        GetScreenWidth()/2.0f,
        GetScreenHeight()/2.0f
    };

    // линия прицеливания
    DrawLineV(center, GetMousePosition(), Fade(WHITE, 0.3f));

    for (float t = 0; t < 1.0f; t += 0.005f)
        DrawCircleV(GetPathPoint(t), 2, LIGHTGRAY);

    for (int i = 0; i < miniChainCount; i++) {
        if (miniChain[i].distance >= 0) {
            Vector2 p = GetPathPoint(miniChain[i].distance / 1000.0f);
            DrawCircleV(p, LETTER_RADIUS, miniChain[i].color);
            DrawText(TextFormat("%c", miniChain[i].charVal),
                     p.x - 5, p.y - 8, 20, WHITE);
        }
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (miniProjectiles[i].active) {
            DrawCircleV(miniProjectiles[i].pos, LETTER_RADIUS, BLUE);
            DrawText(TextFormat("%c", miniProjectiles[i].charVal),
                     miniProjectiles[i].pos.x - 5,
                     miniProjectiles[i].pos.y - 8, 20, WHITE);
        }
    }

    DrawCircleV(center, 30, DARKGREEN);

    DrawText(TextFormat("Next: %c", miniNextBulletChar),
             center.x - 40, center.y + 50, 20, DARKGRAY);

    if (!miniGameFinished) {
        DrawText(TextFormat("LEVEL: %d/%d",
                 miniCurrentLevel + 1, miniTotalLevels),
                 20, 20, 20, DARKGRAY);

        DrawText(TextFormat("Q: %s",
                 miniLevels[miniCurrentLevel].question),
                 20, 50, 20, MAROON);

        DrawText(TextFormat("TARGET: %s",
                 miniLevels[miniCurrentLevel].answer),
                 20, 80, 20, DARKBLUE);
    }

    if (miniGameFinished) {
        DrawRectangle(0, 0,
            GetScreenWidth(), GetScreenHeight(),
            Fade(RAYWHITE, 0.9f));

        DrawText("FINISHED",
                 center.x - 80, center.y, 30, GREEN);

        DrawText("Press ENTER to return",
                 center.x - 140, center.y + 50, 20, DARKGRAY);
    }
}

////////////////////////////////////////////////////////////

/* Convenience check used by the outer level logic. */
bool ShouldExitMinigame(void) {
    return miniGameFinished && IsKeyPressed(KEY_ENTER);
}

/* Expose the score to the rest of the game without leaking internal arrays/state. */
int GetMinigameScore(void) {
    return miniScore;
}

/* Reset long-term minigame progress before starting a completely new run. */
void ResetMinigameProgress(void) {
    miniCurrentLevel = 0;
    miniScore = 0;
    miniGameFinished = false;
}