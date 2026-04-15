/* Level 4 scene logic.
   Plays a short setup dialogue and then launches the Zuma-style minigame before choosing an ending. */

#include "level4.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include "zuma.h"
#include "dialog_scripts.h"

#include <string.h>
#include <stdbool.h>

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



// ENDING EVALUATION

static GameState EvaluateEnding(void)
{
    int wins = angerBad + depressionBad;

    if (wins >= 2)
        return ENDING_GOOD;

    if (wins == 1)
        return ENDING_SLIGHTLY_BAD;

    return ENDING_BAD;
}

// INIT

static void InitLevel4State(void)
{
    waitingOnEvent = false;
    zumaInitialized = false;
    state = LEVEL4_DIALOG;

    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    int count = LEVEL4_TEMPLATE_COUNT;
    CopyScript(LEVEL4_TEMPLATE, count);

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

    if (IsKeyPressed(KEY_ESCAPE) && !IsSettingsMenuOpen())
    {
        OpenPauseMenu();
        return LEVEL4;
    }

    if (state == LEVEL4_DIALOG)
    {
        Texture2D *bg = EventsGetCurrentBackground();
        Texture2D *avatar;

        if (bg && bg->id)
            DrawTexture(*bg, 0, 0, WHITE);
        else
            DrawRectangle(0, 0, w, h, BLACK);

        avatar = EventsGetCurrentAvatar();
        if (avatar && avatar->id)
        {
            float s = 1.5f;
            DrawTexturePro(*avatar,
                (Rectangle){0,0,avatar->width,avatar->height},
                (Rectangle){w - avatar->width*s - 50, h - avatar->height*s,
                            avatar->width*s, avatar->height*s},
                (Vector2){0,0}, 0, WHITE);
        }

        if (IsSettingsMenuOpen())
        {
            EventsDrawOverlay();
            {
                SettingsResult settings = UpdateAndDrawSettingsMenu();
                if (settings == SETTINGS_RESULT_GO_TO_MENU)
                {
                    SaveGameForState(LEVEL4);
                    return MENU;
                }
                if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
            }
            return LEVEL4;
        }

        EventsUpdate();

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
        DrawText("Build each word by shooting letters into the circle.", 20, h - 60, 24, LIGHTGRAY);

        {
            SettingsResult settings = UpdateAndDrawSettingsMenu();
            if (settings == SETTINGS_RESULT_GO_TO_MENU)
            {
                SaveGameForState(LEVEL4);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
            if (settings != SETTINGS_RESULT_NONE) return LEVEL4;
        }

        if (ShouldExitMinigame())
{
    depressionBad = IsMinigameWon() ? 1 : 0;

    finalEnding = EvaluateEnding();
    initialized = false;
    return finalEnding;
}

        return LEVEL4;
    }

    return LEVEL4;
}
