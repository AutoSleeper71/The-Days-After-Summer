#include "zuma.h"
#include "raymath.h"
#include <string.h>
#include <math.h>

#define MAX_LETTERS 100       // max number of the letters on the track
#define MAX_PROJECTILES 20    // max number of the bullets on the screen
#define LETTER_RADIUS 15      // radius of the letter 
#define CHAIN_SPEED 1.2f      // speed of the letters on the track
#define BULLET_SPEED 8.0f     // speed of the bullets
#define LETTER_SPACING 35.0f  // interval between the letters on the track

//letter on the track
typedef struct {
    char charVal;      // letter character
    float distance;    // the distance of the letter on the track
    bool active;       // whether the letter is active
    Color color;       // color of the letter
} Letter;

//bullet the player shoots
typedef struct {
    char charVal;      // letter character
    Vector2 pos;       // current position of the bullet
    Vector2 velocity;  // speed of the bullet
    bool active;       // active status of the bullet
} Projectile;

// question and answer
typedef struct {
    const char* question; // question
    const char* answer;   // answer
} MiniLevelData;

static MiniLevelData miniLevels[] = {
    {"1 + 1 = ?", "TWO"},           // question 1
    {"CAPITAL OF FRANCE?", "PARIS"}, 
    {"OPPOSITE OF HOT?", "COLD"}     // ...
};
static int miniTotalLevels = 3;     // total number of the levels(now 3 )



static int miniCurrentLevel = 0;    // current level index
static int miniScore = 0;           // current score
static Letter miniChain[MAX_LETTERS]; // all letters on the track
static int miniChainCount = 0;      // current letters on the track
static Projectile miniProjectiles[MAX_PROJECTILES]; // all bullets 
static char miniNextBulletChar = 'A'; // next bullet character
static bool miniGameWon = false;    // whether the game is won

static Vector2 GetMinigamePathPoint(float t) {
    float R = 180.0f;  // radius of the track
    float H = 400.0f;  // vertical track length
    float alpha = -2.0f * PI / 3.0f; // track rotation angle (30 degrees tilt)

    float arcLen = PI * R;           // half circle length
    float totalLen = H + arcLen + H; // total track length
    float d = t * totalLen;          // current distance
    Vector2 p = {0, 0};

    // calculate the position of the letter on the track
    // left line -> bottom half circle -> right line
    if (d < H) {
        p.x = -R; p.y = -d;
    } else if (d < H + arcLen) {
        float angle = (d - H) / R;
        p.x = -R * cosf(angle); p.y = -R * sinf(angle);
    } else {
        p.x = R; p.y = (d - H - arcLen);
    }

    // tilt the letter on the track
    float nx = p.x * cosf(alpha) - p.y * sinf(alpha);
    float ny = p.x * sinf(alpha) + p.y * cosf(alpha);

    // shift the position to the center of the screen
    return (Vector2){ nx + GetScreenWidth()/2, ny + GetScreenHeight()/2 + 50 };
}

static void CheckMinigameMatches(void) {
    const char* ans = miniLevels[miniCurrentLevel].answer;
    int ansLen = (int)strlen(ans);
    
    
    // reverse the answer string
    char reversedAns[32];
    for(int i=0; i<ansLen; i++) reversedAns[i] = ans[ansLen - 1 - i];
    reversedAns[ansLen] = '\0';

    // check the matches of the answer string on the track
    for (int i = 0; i <= miniChainCount - ansLen; i++) {
        bool match = true;
        for (int j = 0; j < ansLen; j++) {
            if (miniChain[i + j].charVal != reversedAns[j]) { match = false; break; }
        }
        
        //成功 if the answer string matches the letters on the track
        if (match) {
            // 1. remove the letters from the track
            for (int k = i + ansLen; k < miniChainCount; k++) miniChain[k - ansLen] = miniChain[k];
            miniChainCount -= ansLen;
            
            // 2. increase the score
            miniScore++; 

            // 3. check if the track is empty
            if (miniChainCount <= 0) {
                miniCurrentLevel++;
                // check if all levels are finished
                if (miniCurrentLevel >= miniTotalLevels) miniGameWon = true;
                else InitMinigame(); // load the next level
            }
            return;
        }
    }
}

void InitMinigame(void) {
    if (miniCurrentLevel >= miniTotalLevels) { miniGameWon = true; return; }
    
    ShowCursor();

    miniChainCount = 0;
    miniGameWon = false;
    // clear all bullets
    for(int i=0; i<MAX_PROJECTILES; i++) miniProjectiles[i].active = false;

    // initialize the track with the question
    const char* q = miniLevels[miniCurrentLevel].question;
    for (int i = 0; i < (int)strlen(q); i++) {
        if (q[i] != ' ') {
            miniChain[miniChainCount].charVal = q[i];
            miniChain[miniChainCount].distance = -miniChainCount * LETTER_SPACING;
            miniChain[miniChainCount].active = true;
            miniChain[miniChainCount].color = RED;
            miniChainCount++;
        }
    }
    // set the first bullet character
    miniNextBulletChar = miniLevels[miniCurrentLevel].answer[0];
}

void UpdateMinigame(void) {
    if (miniGameWon) return; // if the game is won,stop the update

    // 1. update the position of the letters on the track
    for (int i = 0; i < miniChainCount; i++) miniChain[i].distance += CHAIN_SPEED;

    // 2. handle the bullet shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (!miniProjectiles[i].active) {
                miniProjectiles[i].active = true;
                // initialize the bullet position
                miniProjectiles[i].pos = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f + 50.0f };
                miniProjectiles[i].charVal = miniNextBulletChar;
                
                // calculate the shooting direction vector
                Vector2 mouse = GetMousePosition();
                Vector2 dir = Vector2Subtract(mouse, miniProjectiles[i].pos);
                miniProjectiles[i].velocity = Vector2Scale(Vector2Normalize(dir), BULLET_SPEED);
                
                // set the next bullet character in the answer string
                const char* ans = miniLevels[miniCurrentLevel].answer;
                int len = (int)strlen(ans);
                
                miniNextBulletChar = ans[GetRandomValue(0, len - 1)];
                break;
            }
        }
    }

    // 3. update the position of the bullets
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (miniProjectiles[i].active) {
            miniProjectiles[i].pos = Vector2Add(miniProjectiles[i].pos, miniProjectiles[i].velocity);
            
            // check if the bullet hits any letter on the track
            for (int j = 0; j < miniChainCount; j++) {
                Vector2 lp = GetMinigamePathPoint(miniChain[j].distance / 1000.0f);
                if (Vector2Distance(miniProjectiles[i].pos, lp) < LETTER_RADIUS * 2) {
                    // hit the letter on the track
                    for (int k = miniChainCount; k > j; k--) miniChain[k] = miniChain[k-1];
                    miniChain[j].charVal = miniProjectiles[i].charVal;
                    miniChain[j].distance = miniChain[j+1].distance - LETTER_SPACING;
                    miniChain[j].color = BLUE;
                    miniChainCount++;
                    
                    miniProjectiles[i].active = false; // destroy the bullet
                    CheckMinigameMatches(); // check the matches
                    break;
                }
            }
            
            // if the bullet is out of the screen boundary,destroy it
            if (miniProjectiles[i].pos.x < 0 || miniProjectiles[i].pos.x > GetScreenWidth() || 
                miniProjectiles[i].pos.y < 0 || miniProjectiles[i].pos.y > GetScreenHeight()) miniProjectiles[i].active = false;
        }
    }
}

void DrawMinigame(void) {
    // 1. draw the track background
    for (float t = 0; t < 1.0f; t += 0.005f) DrawCircleV(GetMinigamePathPoint(t), 2, LIGHTGRAY);
    
    // 2. draw the letters on the track
    for (int i = 0; i < miniChainCount; i++) {
        if (miniChain[i].distance >= 0) {
            Vector2 p = GetMinigamePathPoint(miniChain[i].distance / 1000.0f);
            DrawCircleV(p, LETTER_RADIUS, miniChain[i].color);
            DrawText(TextFormat("%c", miniChain[i].charVal), (int)p.x - 5, (int)p.y - 8, 20, WHITE);
        }
    }

    // 3. draw the bullets
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (miniProjectiles[i].active) {
            DrawCircleV(miniProjectiles[i].pos, LETTER_RADIUS, BLUE);
            DrawText(TextFormat("%c", miniProjectiles[i].charVal), (int)miniProjectiles[i].pos.x - 5, (int)miniProjectiles[i].pos.y - 8, 20, WHITE);
        }
    }

    // 4. draw the turret
    DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2 + 50, 30, DARKGREEN);
    DrawText(TextFormat("Next: %c", miniNextBulletChar), GetScreenWidth()/2 - 30, GetScreenHeight()/2 + 100, 20, DARKGRAY);

    // 5. draw the HUD (score, level, question)
    DrawText(TextFormat("SCORE: %d", miniScore), 20, 20, 30, ORANGE); 
    DrawText(TextFormat("LEVEL: %d/%d", miniCurrentLevel + 1, miniTotalLevels), 20, 60, 20, DARKGRAY);
    DrawText(TextFormat("Q: %s", miniLevels[miniCurrentLevel].question), 20, 90, 20, MAROON);
    DrawText(TextFormat("TARGET: %s", miniLevels[miniCurrentLevel].answer), 20, 120, 20, DARKBLUE);

    // 6. ending screen
    if (miniGameWon) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, 0.8f));
        DrawText("ALL LEVELS CLEARED!", GetScreenWidth()/2 - 150, GetScreenHeight()/2 - 20, 30, GREEN);
        DrawText(TextFormat("FINAL SCORE: %d", miniScore), GetScreenWidth()/2 - 80, GetScreenHeight()/2 + 30, 25, ORANGE);
        DrawText("Press ENTER to continue", GetScreenWidth()/2 - 120, GetScreenHeight()/2 + 80, 20, DARKGRAY);
    }
}
    int GetMinigameScore(void) {
    return miniScore;
    //return the score of the minigame
}

// check if the minigame is finished,used to return to the main game
bool IsMinigameWon(void) { return miniGameWon; }