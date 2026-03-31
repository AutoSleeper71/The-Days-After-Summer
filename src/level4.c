/* Level 4 scene logic.
   Plays a short setup dialogue and then launches the Zuma-style minigame before choosing an ending. */

#include "level4.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include "zuma.h"

#include <string.h>
#include <stdbool.h>

// Helper macro used to get the number of elements in a fixed-size array.
#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
// File-specific compile-time limit used to size arrays safely.
#define MAX_LEVEL4_NODES 64

typedef enum
{
    LEVEL4_START = 0,
    LEVEL4_DIALOG,
    LEVEL4_MINIGAME,
    LEVEL4_FINISH
} Level4State;

static Level4State state = LEVEL4_START;

static bool initialized = false;
static bool waitingOnEvent = false;
static bool zumaInitialized = false;

static DialogState dialog;
static DialogNode nodes[MAX_LEVEL4_NODES];

// UTILITY

/* Copy the level 4 template into a writable node array. */
static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_LEVEL4_NODES) count = MAX_LEVEL4_NODES;
    memcpy(nodes, src, sizeof(DialogNode) * count);
}

/* Catch broken dialogue links before the level starts running. */
static bool ValidateScript(const DialogNode *nodes, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (nodes[i].next < -1 || nodes[i].next >= count)
            return false;

        for (int c = 0; c < nodes[i].choiceCount; c++)
        {
            int t = nodes[i].choices[c].next;
            if (t < 0 || t >= count)
                return false;
        }
    }
    return true;
}


// DIALOG

/* Short setup dialogue before the final minigame. */
static const DialogNode template[] =
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


// ENDING EVALUATION

static GameState EvaluateEnding(void)
{
    int total = angerBad + depressionBad;

    if (total >= 3)
        return ENDING_BAD;

    if (total >= 1)
        return ENDING_SLIGHTLY_BAD;

    return ENDING_GOOD;
}

// INIT

static void InitLevel4State(void)
{
    waitingOnEvent = false;
    zumaInitialized = false;
    state = LEVEL4_START;

    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    int count = ARRAY_COUNT(template);
    CopyScript(template, count);

    if (!ValidateScript(nodes, count))
    {
        TraceLog(LOG_ERROR, "Level4 script invalid");
        initialized = false;
        return;
    }

    DialogStart(&dialog, nodes);
    initialized = true;
}

void InitLevel4(void)
{
    initialized = false;
}


// =========================
// UPDATE
// =========================

GameState UpdateLevel4(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if (!initialized)
        InitLevel4State();

    if (!initialized)
        return LEVEL4;


    // =========================
    // START SCREEN
    // =========================
    if (state == LEVEL4_START)
    {
        DrawText("LEVEL 4 - DEPRESSION", w/2 - 220, h/2 - 80, 40, WHITE);

        DrawText("ENTER = try to fight this", w/2 - 120, h/2, 20, WHITE);
        DrawText("SPACE = give up", w/2 - 120, h/2 + 40, 20, GRAY);

        if (IsKeyPressed(KEY_SPACE))
        {
            // skip = heavy penalty
            depressionBad += 2;

            finalEnding = EvaluateEnding();
            nextLevel = finalEnding;

            initialized = false;
            return ELEVATOR;
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            state = LEVEL4_DIALOG;
        }

        return LEVEL4;
    }


    // =========================
    // DIALOG
    // =========================
    if (state == LEVEL4_DIALOG)
    {
        EventsUpdate();

        Texture2D *bg = EventsGetCurrentBackground();
        if (bg && bg->id)
            DrawTexture(*bg, 0, 0, WHITE);

        Texture2D *avatar = EventsGetCurrentAvatar();
        if (avatar && avatar->id)
        {
            float s = 1.5f;
            DrawTexturePro(*avatar,
                (Rectangle){0,0,avatar->width,avatar->height},
                (Rectangle){w - avatar->width*s - 50, h - avatar->height*s,
                            avatar->width*s, avatar->height*s},
                (Vector2){0,0}, 0, WHITE);
        }

        if (waitingOnEvent && !dialog.finished && !EventsBusy())
        {
            waitingOnEvent = false;
            DialogResume(&dialog);
        }

        if (!dialog.finished)
        {
            if (!waitingOnEvent)
            {
                DialogEvent ev = DialogUpdate(&dialog);

                if (ev != EVENT_NONE)
                {
                    DialogNode *n = &dialog.nodes[dialog.index];
                    EventsTrigger(ev, n->backgroundId, n->avatarId, n->soundId, n->inspectId);
                    waitingOnEvent = true;
                }
            }

            if (EventsIsDialogVisible())
                DialogDraw(&dialog);
        }
        else
        {
            state = LEVEL4_MINIGAME;
        }

        EventsDrawOverlay();
        return LEVEL4;
    }


    // MINIGAME ITSELF
    
if (state == LEVEL4_MINIGAME)
{
    if (!zumaInitialized)
    {
        ResetMinigameProgress();
        InitMinigame();
        zumaInitialized = true;
    }

    UpdateMinigame();
    DrawMinigame();

    DrawText("Clear the chain.", 20, h - 60, 24, LIGHTGRAY);

    // НОВАЯ логика выхода
    if (ShouldExitMinigame())
    {
        int score = GetMinigameScore();

        if (score <= 1) depressionBad += 2;
        else if (score == 2) depressionBad += 1;

        finalEnding = EvaluateEnding();
        nextLevel = finalEnding;

        initialized = false;
        return ELEVATOR;
    }

    return LEVEL4;
}

    return LEVEL4;
}