#include "level4.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include "zuma.h"

#include <string.h>
#include <stdbool.h>

#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define MAX_LEVEL4_NODES 64

typedef enum
{
    LEVEL4_PHASE_DIALOG = 0,
    LEVEL4_PHASE_ZUMA
} Level4Phase;

static bool level4Initialized = false;
static DialogState level4Dialog;
static DialogNode activeNodes[MAX_LEVEL4_NODES];
static bool waitingOnEvent = false;
static Level4Phase level4Phase = LEVEL4_PHASE_DIALOG;
static bool zumaInitialized = false;

static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_LEVEL4_NODES) count = MAX_LEVEL4_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}

static bool ValidateScript(const DialogNode *nodes, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (nodes[i].next < -1 || nodes[i].next >= count)
            return false;

        if (nodes[i].choiceCount < 0 || nodes[i].choiceCount > MAX_CHOICES)
            return false;

        for (int c = 0; c < nodes[i].choiceCount; c++)
        {
            int target = nodes[i].choices[c].next;
            if (target < 0 || target >= count)
                return false;
        }
    }
    return true;
}

static const DialogNode level4Template[] =
{
    { "Narration", "The next floor greets you with a quiet that feels almost kind.",
      EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_INSIDE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 1 },

    { "Daniel", "No shouting. No panic. Just... emptiness.",
      EVENT_AVATAR_SHOW, BG_NONE, AVATAR_NEUTRAL, SOUND_NONE,
      INSPECT_NONE, 0, {}, 2 },

    { "Narration", "A final memory drifts in front of you, asking only one thing.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 3 },

    { "Narration", "Can you still reach for an answer, even now?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 4 },

    { "Narration", "You step forward.",
      EVENT_FADE_OUT | EVENT_AVATAR_HIDE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, -1 }
};

static void InitLevel4State(void)
{
    int count = ARRAY_COUNT(level4Template);

    waitingOnEvent = false;
    level4Phase = LEVEL4_PHASE_DIALOG;
    zumaInitialized = false;

    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    CopyScript(level4Template, count);

    if (!ValidateScript(activeNodes, count))
    {
        TraceLog(LOG_ERROR, "Level4 dialog script has invalid indices.");
        level4Initialized = false;
        return;
    }

    DialogStart(&level4Dialog, activeNodes);
    level4Initialized = true;
}

static GameState EvaluateEnding(void)
{
    if (angerBad && depressionBad)
        return ENDING_BAD;

    if (depressionBad)
        return ENDING_SLIGHTLY_BAD;

    return ENDING_GOOD;
}

void InitLevel4(void)
{
    level4Initialized = false;
}

GameState UpdateLevel4(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if (!level4Initialized)
        InitLevel4State();

    if (!level4Initialized)
    {
        DrawRectangle(0, 0, w, h, BLACK);
        DrawText("Level 4 failed to initialize", 40, 40, 24, RED);
        return LEVEL4;
    }

    if (level4Phase == LEVEL4_PHASE_DIALOG)
    {
        EventsUpdate();

        Texture2D *bg = EventsGetCurrentBackground();
        if (bg && bg->id != 0)
            DrawTexture(*bg, 0, 0, WHITE);
        else
            DrawRectangle(0, 0, w, h, BLACK);

        Texture2D *avatar = EventsGetCurrentAvatar();
        if (avatar && avatar->id != 0)
        {
            float scale = 1.5f;
            DrawTexturePro(
                *avatar,
                (Rectangle){0, 0, (float)avatar->width, (float)avatar->height},
                (Rectangle){
                    w - avatar->width * scale - 50,
                    h - avatar->height * scale,
                    avatar->width * scale,
                    avatar->height * scale
                },
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        }

        if (waitingOnEvent && !level4Dialog.finished && !EventsBusy())
        {
            waitingOnEvent = false;
            DialogResume(&level4Dialog);
        }

        if (!level4Dialog.finished)
        {
            if (EventsShouldBlockInput())
                return LEVEL4;

            if (!waitingOnEvent)
            {
                DialogEvent ev = DialogUpdate(&level4Dialog);

                if (ev != EVENT_NONE)
                {
                    DialogNode *node = &level4Dialog.nodes[level4Dialog.index];
                    EventsTrigger(ev, node->backgroundId, node->avatarId, node->soundId, node->inspectId);
                    waitingOnEvent = true;
                }
            }

            if (EventsIsDialogVisible())
                DialogDraw(&level4Dialog);
        }
        else
        {
            level4Phase = LEVEL4_PHASE_ZUMA;
        }

        EventsDrawOverlay();
        return LEVEL4;
    }

    if (!zumaInitialized)
    {
        ResetMinigameProgress();
        InitMinigame();
        zumaInitialized = true;
    }

    UpdateMinigame();
    DrawMinigame();

    DrawText("LEVEL 4 - DEPRESSION", 20, h - 140, 34, WHITE);
    DrawText("Clear the memory chain to reach the ending.", 20, h - 100, 24, LIGHTGRAY);

    if (IsMinigameWon())
    {
        DrawText("PRESS ENTER TO FINISH", w/2 - 150, h - 50, 20, YELLOW);

        if (IsKeyPressed(KEY_ENTER))
        {
            int finalScore = GetMinigameScore();

            if (finalScore <= 1)
                depressionBad += 2;
            else if (finalScore == 2)
                depressionBad += 1;

            finalEnding = EvaluateEnding();
            nextLevel = finalEnding;

            level4Initialized = false;
            zumaInitialized = false;
            return ELEVATOR;
        }
    }

    return LEVEL4;
}
