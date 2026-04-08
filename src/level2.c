/* Level 2 scene logic.
   Replays the relationship memory with different emotional beats and branching responses. */

#include "level2.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include "dialog_scripts.h"

#include <string.h>
#include <stdbool.h>


// File-specific compile-time limit used to size arrays safely.
#define MAX_LEVEL2_NODES 220
#define LEVEL2_START_FADE_TIME 2.0f

static bool level2Initialized = false;
static DialogState level2Dialog;
static DialogNode activeNodes[MAX_LEVEL2_NODES];
static bool waitingOnEvent = false;
static bool introFadeActive = false;
static float introFadeTimer = 0.0f;

/* Copy the level 2 template into a writable runtime buffer. */
static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_LEVEL2_NODES) count = MAX_LEVEL2_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}




/* Guard against invalid next-node indices in the large branching script. */
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


/* Build runtime data for level 2 and reset shared scene effects. */
static void InitLevel2State(void)
{
    int count = LEVEL2_TEMPLATE_COUNT;
    CopyScript(LEVEL2_TEMPLATE, count);

    waitingOnEvent = false;
    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    if (!ValidateScript(activeNodes, count))
    {
        TraceLog(LOG_ERROR, "Level2 dialog script has invalid indices.");
        level2Initialized = false;
        return;
    }

    DialogStart(&level2Dialog, activeNodes);

    introFadeActive = true;
    introFadeTimer = 0.0f;

    level2Initialized = true;
}

void InitLevel2(void)
{
    level2Initialized = false;
}

GameState UpdateLevel2(void)
{
    if (!level2Initialized)
        InitLevel2State();

    if (!level2Initialized)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
        DrawText("Level 2 failed to initialize", 40, 40, 24, RED);
        return LEVEL2;
    }

    if (IsKeyPressed(KEY_ESCAPE) && !IsSettingsMenuOpen())
    {
        OpenPauseMenu();
        return LEVEL2;
    }

    {
        Texture2D *bg = EventsGetCurrentBackground();
        if (bg && bg->id != 0)
            DrawTexture(*bg, 0, 0, WHITE);
        else
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    }

    {
        Texture2D *avatar = EventsGetCurrentAvatar();
        if (avatar && avatar->id != 0)
        {
            float scale = 1.5f;
            DrawTexturePro(
                *avatar,
                (Rectangle){0, 0, (float)avatar->width, (float)avatar->height},
                (Rectangle){
                    GetScreenWidth() - avatar->width * scale - 50,
                    GetScreenHeight() - avatar->height * scale,
                    avatar->width * scale,
                    avatar->height * scale
                },
                (Vector2){0, 0},
                0.0f,
                WHITE
            );
        }
    }

    if (IsSettingsMenuOpen())
    {
        EventsDrawOverlay();
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.08f));

        {
            SettingsResult settings = UpdateAndDrawSettingsMenu();
            if (settings == SETTINGS_RESULT_GO_TO_MENU)
            {
                SaveGameForState(LEVEL2);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
        }
        return LEVEL2;
    }

    EventsUpdate();

    {
        GameState requestedState;
        if (EventsConsumeTransition(&requestedState))
        {
            level2Initialized = false;
            if (requestedState == LEVEL3) level3Unlocked = 1;
            return requestedState;
        }
    }

    if (introFadeActive)
    {
        float fadeAlpha;
        introFadeTimer += GetFrameTime();
        fadeAlpha = 1.0f - (introFadeTimer / LEVEL2_START_FADE_TIME);
        if (fadeAlpha < 0.0f) fadeAlpha = 0.0f;

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fadeAlpha));

        if (introFadeTimer >= LEVEL2_START_FADE_TIME)
            introFadeActive = false;

        EventsDrawOverlay();
        return LEVEL2;
    }

    if (waitingOnEvent && !level2Dialog.finished && !EventsBusy())
    {
        waitingOnEvent = false;
        DialogResume(&level2Dialog);
    }

    if (!level2Dialog.finished)
    {
        if (EventsShouldBlockInput())
        {
            EventsDrawOverlay();
            return LEVEL2;
        }

        if (!waitingOnEvent)
        {
            DialogEvent ev = DialogUpdate(&level2Dialog);
            if (ev != EVENT_NONE)
            {
                DialogNode *node = &level2Dialog.nodes[level2Dialog.index];
                EventsTrigger(ev, node->backgroundId, node->avatarId, node->soundId, node->inspectId);
                waitingOnEvent = true;
            }
        }

        if (EventsIsDialogVisible())
            DialogDraw(&level2Dialog);
    }

    EventsDrawOverlay();
    return LEVEL2;
}
