/* Level 1 scene logic.
   Builds the dialogue script for the first memory and runs its intro, events, and transitions. */

#include "level1.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include "dialog_scripts.h"

#include <string.h>
#include <stdbool.h>


// File-specific compile-time limit used to size arrays safely.
#define MAX_LEVEL1_NODES 220
#define LEVEL1_START_FADE_TIME 2.0f

static bool level1Initialized = false;
static DialogState level1Dialog;
static DialogNode activeNodes[MAX_LEVEL1_NODES];
static bool waitingOnEvent = false;
static bool introFadeActive = false;
static float introFadeTimer = 0.0f;

/* Copy the level script into a writable array.
   Dialogue nodes are copied because some event flags are cleared after first use. */
static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_LEVEL1_NODES) count = MAX_LEVEL1_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}

/* Helper used to decide which portrait should be shown for a given dialogue line. */
static bool IsProtagonistSpeaker(const char *speaker)
{
    return (strcmp(speaker, "You") == 0) ||
           (strcmp(speaker, "YOU") == 0) ||
           (strcmp(speaker, "Daniel") == 0);
}

static bool IsGirlSpeaker(const char *speaker)
{
    return (strcmp(speaker, "Her") == 0) ||
           (strcmp(speaker, "HER") == 0);
}

static int AvatarForSpeaker(const char *speaker)
{
    if (IsProtagonistSpeaker(speaker)) return AVATAR_NEUTRAL;
    if (IsGirlSpeaker(speaker)) return AVATAR_GIRL_HAPPY;
    return AVATAR_NONE;
}

/* preload only the NEXT avatar show so there are no blank avatar-change lines */
/* Preload the avatar that will speak next.
   This avoids a blank frame when the speaker changes. */
static void ApplyAvatarPreload(DialogNode *nodes, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        DialogNode *cur = &nodes[i];
        DialogNode *next = &nodes[i + 1];

        int nextAvatar = AvatarForSpeaker(next->speaker);

        if (nextAvatar != AVATAR_NONE)
        {
            if ((cur->event & EVENT_AVATAR_HIDE) == 0)
            {
                cur->event |= EVENT_AVATAR_SHOW;
                cur->avatarId = nextAvatar;
            }
        }
    }
}

/* Basic safety check so broken node indices are caught early instead of causing crashes. */
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



/* Build runtime data for level 1 and apply a short manual fade before dialogue begins. */
static void InitLevel1State(void)
{
    int count = LEVEL1_TEMPLATE_COUNT;
    CopyScript(LEVEL1_TEMPLATE, count);

    waitingOnEvent = false;
    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    ApplyAvatarPreload(activeNodes, count);

    if (!ValidateScript(activeNodes, count))
    {
        TraceLog(LOG_ERROR, "Level1 dialog script has invalid indices.");
        level1Initialized = false;
        return;
    }

    DialogStart(&level1Dialog, activeNodes);

    introFadeActive = true;
    introFadeTimer = 0.0f;

    level1Initialized = true;
}

void InitLevel1(void)
{
    level1Initialized = false;
}

GameState UpdateLevel1(void)
{
    if (!level1Initialized)
        InitLevel1State();

    if (!level1Initialized)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
        return LEVEL1;
    }

    if (IsKeyPressed(KEY_ESCAPE) && !IsSettingsMenuOpen())
    {
        OpenPauseMenu();
        return LEVEL1;
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
                SaveGameForState(LEVEL1);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
        }
        return LEVEL1;
    }

    EventsUpdate();

    {
        GameState requestedState;
        if (EventsConsumeTransition(&requestedState))
        {
            level1Initialized = false;
            if (requestedState == LEVEL2) level2Unlocked = 1;
            return requestedState;
        }
    }

    if (introFadeActive)
    {
        float fadeAlpha;
        introFadeTimer += GetFrameTime();
        fadeAlpha = 1.0f - (introFadeTimer / LEVEL1_START_FADE_TIME);
        if (fadeAlpha < 0.0f) fadeAlpha = 0.0f;

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fadeAlpha));

        if (introFadeTimer >= LEVEL1_START_FADE_TIME)
            introFadeActive = false;

        EventsDrawOverlay();
        return LEVEL1;
    }

    if (waitingOnEvent && !level1Dialog.finished && !EventsBusy())
    {
        waitingOnEvent = false;
        DialogResume(&level1Dialog);
    }

    if (!level1Dialog.finished)
    {
        if (EventsShouldBlockInput())
        {
            EventsDrawOverlay();
            return LEVEL1;
        }

        if (!waitingOnEvent)
        {
            DialogEvent ev = DialogUpdate(&level1Dialog);
            if (ev != EVENT_NONE)
            {
                DialogNode *node = &level1Dialog.nodes[level1Dialog.index];
                EventsTrigger(ev, node->backgroundId, node->avatarId, node->soundId, node->inspectId);
                waitingOnEvent = true;
            }
        }

        if (EventsIsDialogVisible())
            DialogDraw(&level1Dialog);
    }

    EventsDrawOverlay();
    return LEVEL1;
}
