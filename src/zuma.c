/* Zuma-style word-building minigame for level 4.
   The player shoots from the center toward one moving target.
   Each correct hit fills the next letter of the current word at the bottom.
   The minigame starts with an instructions page and gives 3 full attempts.
   A dedicated background track plays only while this minigame is active. */

#include "zuma.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

#define TOTAL_WORDS 3
#define MAX_ATTEMPTS 3
#define MAX_MISSES_PER_RUN 6
#define SHOT_SPEED 780.0f
#define SHOT_RADIUS 20.0f
#define TARGET_RADIUS 60.0f
#define ORBIT_RADIUS 350.0f
#define ORBIT_SPEED 1.1f
#define MODE_SWITCH_TIME 3.0f
#define TARGET_JUMP_INTERVAL 0.75f
#define AIM_SMOOTHING 0.18f
#define MUSIC_PATH "assets/audio/zumasong.mp3"
static float exitTimer = 0.0f;

typedef enum
{
    MINIGAME_INSTRUCTIONS = 0,
    MINIGAME_PLAYING,
    MINIGAME_FINISHED
} MinigamePhase;

typedef enum
{
    TARGET_MODE_ORBIT = 0,
    TARGET_MODE_WANDER
} TargetMode;

typedef struct
{
    const char *prompt;
    const char *answer;
} WordRound;

typedef struct
{
    char letter;
    Vector2 pos;
    Vector2 vel;
    bool active;
} Shot;

static const WordRound rounds[TOTAL_WORDS] = {
    {"I think I am just dragging you down with me", "COMFORT"},
    {"I think it is better if I step away for a while. It is just too much right now", "STABILITY"},
    {"Nothing will change.", "HOPE"}
};

static MinigamePhase phase = MINIGAME_INSTRUCTIONS;
static TargetMode targetMode = TARGET_MODE_ORBIT;
static Shot shot = {0};

static int currentRound = 0;
static int builtCount = 0;
static int attemptsLeft = MAX_ATTEMPTS;
static int missesThisRun = 0;
static int miniScore = 0;
static bool miniGameWon = false;
static bool exitReady = false;

static float targetAngle = -PI/2.0f;
static float targetWanderAngle = -PI/2.0f;
static float modeTimer = 0.0f;
static float targetJumpTimer = 0.0f;

static Vector2 aimDir = { 1.0f, 0.0f };

static Music zumaMusic = {0};
static bool musicLoaded = false;
static bool musicPlaying = false;

static Vector2 GetCenter(void)
{
    return (Vector2){ GetScreenWidth() * 0.50f, GetScreenHeight() * 0.50f };
}

static const char *GetCurrentAnswer(void)
{
    return rounds[currentRound].answer;
}

static int GetCurrentAnswerLen(void)
{
    return (int)strlen(GetCurrentAnswer());
}

static char GetNextLetter(void)
{
    return GetCurrentAnswer()[builtCount];
}

static void ClearShot(void)
{
    shot.active = false;
    shot.letter = '\0';
    shot.pos = (Vector2){0};
    shot.vel = (Vector2){0};
}

static void EnsureMusicLoaded(void)
{
    if (musicLoaded) return;

    if (FileExists(MUSIC_PATH))
    {
        zumaMusic = LoadMusicStream(MUSIC_PATH);
        musicLoaded = (zumaMusic.ctxData != NULL);
    }
}

static void StartZumaMusic(void)
{
    EnsureMusicLoaded();

    if (musicLoaded && !musicPlaying)
    {
        SetMusicVolume(zumaMusic, 1.0f);
        PlayMusicStream(zumaMusic);
        musicPlaying = true;
    }
}

static void StopZumaMusic(void)
{
    if (musicLoaded && musicPlaying)
    {
        StopMusicStream(zumaMusic);
        musicPlaying = false;
    }
}

static void UpdateZumaMusic(void)
{
    if (musicLoaded && musicPlaying)
        UpdateMusicStream(zumaMusic);
}

static Vector2 GetTargetPosition(void)
{
    Vector2 center = GetCenter();
    return (Vector2){
        center.x + cosf(targetAngle) * ORBIT_RADIUS,
        center.y + sinf(targetAngle) * ORBIT_RADIUS
    };
}

static void ResetTargetMotion(void)
{
    targetAngle = -PI/2.0f;
    targetWanderAngle = targetAngle;
    targetMode = TARGET_MODE_ORBIT;
    modeTimer = 0.0f;
    targetJumpTimer = 0.0f;
}

static void ResetAimDirection(void)
{
    aimDir = (Vector2){ 1.0f, 0.0f };
}

static void StartRunFromBeginning(void)
{
    currentRound = 0;
    builtCount = 0;
    missesThisRun = 0;
    miniScore = 0;
    miniGameWon = false;
    exitReady = false;
    phase = MINIGAME_PLAYING;
    ClearShot();
    ResetTargetMotion();
    ResetAimDirection();
}

static void StartNextRound(void)
{
    builtCount = 0;
    phase = MINIGAME_PLAYING;
    ClearShot();
    ResetTargetMotion();
    ResetAimDirection();
}

static void LoseAttemptAndMaybeFinish(void)
{
    attemptsLeft--;
    ClearShot();

    if (attemptsLeft <= 0)
    {
        attemptsLeft = 0;
        miniGameWon = false;
        exitReady = true;
        phase = MINIGAME_FINISHED;
        StopZumaMusic();
        return;
    }

    StartRunFromBeginning();
}

static void UpdateAimDirection(void)
{
    Vector2 center = GetCenter();
    Vector2 mouse = GetMousePosition();
    Vector2 rawDir = Vector2Subtract(mouse, center);

    if (Vector2Length(rawDir) > 0.001f)
    {
        rawDir = Vector2Normalize(rawDir);
        aimDir = Vector2Normalize(Vector2Lerp(aimDir, rawDir, AIM_SMOOTHING));
    }
}

static void FireShot(void)
{
    Vector2 start = GetCenter();

    if (Vector2Length(aimDir) <= 0.001f)
        return;

    shot.active = true;
    shot.letter = GetNextLetter();
    shot.pos = start;
    shot.vel = Vector2Scale(aimDir, SHOT_SPEED);
}

static void UpdateTarget(void)
{
    float dt = GetFrameTime();
    modeTimer += dt;

    if (modeTimer >= MODE_SWITCH_TIME)
    {
        modeTimer = 0.0f;

        if (targetMode == TARGET_MODE_ORBIT)
        {
            targetMode = TARGET_MODE_WANDER;
            targetWanderAngle = GetRandomValue(0, 628) / 100.0f;
            targetJumpTimer = 0.0f;
        }
        else
        {
            targetMode = TARGET_MODE_ORBIT;
        }
    }

    if (targetMode == TARGET_MODE_ORBIT)
    {
        targetAngle += ORBIT_SPEED * dt;
    }
    else
    {
        float diff;

        targetJumpTimer += dt;
        if (targetJumpTimer >= TARGET_JUMP_INTERVAL)
        {
            targetJumpTimer = 0.0f;
            targetWanderAngle = GetRandomValue(0, 628) / 100.0f;
        }

        diff = targetWanderAngle - targetAngle;
        while (diff > PI) diff -= 2.0f * PI;
        while (diff < -PI) diff += 2.0f * PI;

        targetAngle += diff * 3.2f * dt;
    }
}

static void UpdateShot(void)
{
    Vector2 targetPos;
    float dt;

    if (!shot.active) return;

    targetPos = GetTargetPosition();
    dt = GetFrameTime();
    shot.pos = Vector2Add(shot.pos, Vector2Scale(shot.vel, dt));

    if (CheckCollisionCircles(shot.pos, SHOT_RADIUS, targetPos, TARGET_RADIUS))
    {
        builtCount++;
        miniScore = currentRound;
        ClearShot();

        if (builtCount >= GetCurrentAnswerLen())
        {
            miniScore = currentRound + 1;
            currentRound++;

            if (currentRound >= TOTAL_WORDS)
            {
                miniGameWon = true;
                phase = MINIGAME_FINISHED;
                exitTimer = 0.0f;
                StopZumaMusic();
            }
            else
            {
                StartNextRound();
            }
        }
        return;
    }

    if (shot.pos.x < -60.0f || shot.pos.x > GetScreenWidth() + 60.0f ||
        shot.pos.y < -60.0f || shot.pos.y > GetScreenHeight() + 60.0f)
    {
        missesThisRun++;
        ClearShot();

        if (missesThisRun >= MAX_MISSES_PER_RUN)
            LoseAttemptAndMaybeFinish();
    }
}

static void DrawCenteredText(const char *text, int y, int size, Color color)
{
    int x = GetScreenWidth()/2 - MeasureText(text, size)/2;
    DrawText(text, x, y, size, color);
}

static void DrawInstructionsScreen(void)
{
    int w = GetScreenWidth();
    int x = 130;
    int y = 110;
    float boxHeight = GetScreenHeight() - 140.0f;

    ClearBackground((Color){18, 16, 22, 255});

    DrawRectangleRounded((Rectangle){90, 70, w - 180.0f, boxHeight}, 0.05f, 10, Fade(BLACK, 0.65f));
    DrawRectangleRoundedLinesEx((Rectangle){90, 70, w - 180.0f, boxHeight}, 0.05f, 10, 2.0f, RAYWHITE);

    DrawText("WORD LINK", x, y, 52, RAYWHITE);
    y += 90;

    DrawText("Shoot from the center and hit the moving target.", x, y, 28, LIGHTGRAY);
    y += 45;
    DrawText("Each hit places the next letter into the word at the bottom.", x, y, 28, LIGHTGRAY);
    y += 45;
    DrawText("You get 3 total attempts.", x, y, 28, LIGHTGRAY);
    y += 45;
    DrawText("If you miss too many shots, the whole minigame restarts from word 1.", x, y, 28, LIGHTGRAY);
    y += 70;

    DrawText("Words for this stage:", x, y, 32, RAYWHITE);
    y += 55;

    DrawText("1) COMFORT", x + 30, y, 34, SKYBLUE); y += 50;
    DrawText("2) STABILITY", x + 30, y, 34, SKYBLUE); y += 50;
    DrawText("3) HOPE", x + 30, y, 34, SKYBLUE); y += 80;

    DrawText("Controls:", x, y, 32, RAYWHITE);
    y += 55;
    DrawText("Move mouse to aim", x + 30, y, 30, LIGHTGRAY); y += 42;
    DrawText("Left click to shoot", x + 30, y, 30, LIGHTGRAY); y += 42;
    DrawText("ESC for pause menu", x + 30, y, 30, LIGHTGRAY); y += 95;

    DrawCenteredText("Press ENTER to begin", (int)(70 + boxHeight - 60.0f), 34, YELLOW);
}

static void DrawTopLeftHud(void)
{
    Rectangle panel = { 28, 24, 830, 170 };

    DrawRectangleRounded(panel, 0.12f, 10, Fade(BLACK, 0.70f));
    DrawRectangleRoundedLinesEx(panel, 0.12f, 10, 2.0f, RAYWHITE);

    DrawText(TextFormat("WORD %d / %d", currentRound + 1, TOTAL_WORDS), 48, 42, 28, RAYWHITE);
    DrawText(TextFormat("Arrange: %s", GetCurrentAnswer()), 48, 82, 34, YELLOW);
    DrawText(TextFormat("Attempts Left: %d", attemptsLeft), 48, 126, 26, LIGHTGRAY);
    DrawText(TextFormat("Misses This Run: %d / %d", missesThisRun, MAX_MISSES_PER_RUN), 320, 126, 26, LIGHTGRAY);
}

static void DrawBottomWord(void)
{
    const char *answer = GetCurrentAnswer();
    int len = GetCurrentAnswerLen();
    int boxSize = 56;
    int gap = 12;
    int totalWidth = len * boxSize + (len - 1) * gap;
    int startX = GetScreenWidth()/2 - totalWidth/2;
    int y = GetScreenHeight() - 145;

    for (int i = 0; i < len; i++)
    {
        Rectangle r = { (float)(startX + i * (boxSize + gap)), (float)y, (float)boxSize, (float)boxSize };
        Color border = (i == builtCount) ? YELLOW : RAYWHITE;

        DrawRectangleRounded(r, 0.18f, 8, Fade(BLACK, 0.62f));
        DrawRectangleRoundedLinesEx(r, 0.18f, 8, 2.0f, border);

        if (i < builtCount)
        {
            char text[2] = { answer[i], '\0' };
            DrawText(text, (int)r.x + 18, (int)r.y + 10, 34, SKYBLUE);
        }
    }
}

static void DrawTargetAndShooter(void)
{
    Vector2 center = GetCenter();
    Vector2 target = GetTargetPosition();

    DrawLineEx(center, Vector2Add(center, Vector2Scale(aimDir, 68.0f)), 8.0f, Fade(RAYWHITE, 0.55f));
    DrawCircleV(center, 34.0f, DARKGREEN);
    DrawCircleLines((int)center.x, (int)center.y, 34.0f, RAYWHITE);

    if (targetMode == TARGET_MODE_ORBIT)
    {
        DrawCircleV(target, TARGET_RADIUS, MAROON);
        DrawCircleLines((int)target.x, (int)target.y, TARGET_RADIUS, YELLOW);
    }
    else
    {
        DrawCircleV(target, TARGET_RADIUS, DARKPURPLE);
        DrawCircleLines((int)target.x, (int)target.y, TARGET_RADIUS, SKYBLUE);
    }

    DrawText("TARGET", (int)target.x - 45, (int)target.y - 10, 22, WHITE);

    if (shot.active)
    {
        char letter[2] = { shot.letter, '\0' };
        DrawCircleV(shot.pos, SHOT_RADIUS, BLUE);
        DrawCircleLines((int)shot.pos.x, (int)shot.pos.y, SHOT_RADIUS, RAYWHITE);
        DrawText(letter, (int)shot.pos.x - 9, (int)shot.pos.y - 14, 30, WHITE);
    }
    else
    {
        char letter[2] = { GetNextLetter(), '\0' };
        DrawText(TextFormat("Next: %s", letter), (int)center.x - 45, (int)center.y + 52, 24, LIGHTGRAY);
    }
}


static void DrawCustomCursor(void)
{
    Vector2 mouse = GetMousePosition();

    DrawCircleLines((int)mouse.x, (int)mouse.y, 18, WHITE);
    DrawCircle((int)mouse.x, (int)mouse.y, 4, WHITE);
    DrawLine((int)(mouse.x - 10), (int)mouse.y, (int)(mouse.x + 10), (int)mouse.y, WHITE);
    DrawLine((int)mouse.x, (int)(mouse.y - 10), (int)mouse.x, (int)(mouse.y + 10), WHITE);
}

static void DrawFinishOverlay(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.48f));

    if (miniGameWon)
    {
        DrawCenteredText("All words complete!", GetScreenHeight()/2 - 40, 42, GREEN);
        DrawCenteredText("You are ready to move on.", GetScreenHeight()/2 + 20, 28, RAYWHITE);
    }
    else
    {
        DrawCenteredText("Out of attempts", GetScreenHeight()/2 - 40, 42, RED);
        DrawCenteredText("The minigame will now end.", GetScreenHeight()/2 + 20, 28, RAYWHITE);
    }
}

void ResetMinigameProgress(void)
{
    currentRound = 0;
    builtCount = 0;
    attemptsLeft = MAX_ATTEMPTS;
    missesThisRun = 0;
    miniScore = 0;
    miniGameWon = false;
    exitReady = false;
    phase = MINIGAME_INSTRUCTIONS;
    ClearShot();
    ResetTargetMotion();
    ResetAimDirection();
    StopZumaMusic();
}

void InitMinigame(void)
{
    ClearShot();
    ResetTargetMotion();
    ResetAimDirection();
    StartZumaMusic();
}

void UpdateMinigame(void)
{
    UpdateZumaMusic();

    if (phase == MINIGAME_INSTRUCTIONS)
    {
        if (IsKeyPressed(KEY_ENTER))
            StartRunFromBeginning();
        return;
    }

    if (phase == MINIGAME_PLAYING)
    {
        UpdateAimDirection();
        UpdateTarget();

        if (!shot.active && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            FireShot();

        UpdateShot();
        return;
    }

    if (phase == MINIGAME_FINISHED)
{
    exitTimer += GetFrameTime();

    if (exitTimer > 0.5f) // small buffer (0.3–1.0 works)
        exitReady = true;

    return;
}
}

void DrawMinigame(void)
{
    if (phase == MINIGAME_INSTRUCTIONS)
    {
        DrawInstructionsScreen();
        DrawCustomCursor();
        return;
    }

    ClearBackground((Color){22, 20, 28, 255});

    if (phase == MINIGAME_FINISHED)
    {
        DrawFinishOverlay();
        DrawCustomCursor();
        return;
    }

    DrawTopLeftHud();
    DrawBottomWord();
    DrawTargetAndShooter();
    DrawCustomCursor();
}

bool IsMinigameWon(void)
{
    return miniGameWon;
}

int GetMinigameScore(void)
{
    return miniScore;
}

bool ShouldExitMinigame(void)
{
    return exitReady;
}
