/* Ending controller.
   Selects one of several ending scripts and plays it through the shared dialogue/event system. */

#include "ending.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include <string.h>

// File-specific compile-time limit used to size arrays safely.
#define MAX_ENDING_NODES 64

static DialogState endingDialog;
static DialogNode endingNodes[MAX_ENDING_NODES];

/* controls flow */
static bool waitingOnEvent = false;

// SCRIPT

static const DialogNode goodEndingTemplate[] =
{
    { // 0
        "Narration",
        "Six months have passed since the incident.",
        EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_PLAY_SOUND,
        BG_HAPPY_ENDING, AVATAR_NONE, SOUND_GOOD_END,
        INSPECT_NONE,
        0, {}, 1
    },

    { // 1
        "Narration",
        "Life is uneventful, but…",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 2
    },

    { // 2
        "Narration",
        "That's okay.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 3
    },

    { // 3
        "Narration",
        "Even though you're still staying in your small apartment, and going to that same job.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },

    { // 4
        "Narration",
        "You try your best to work towards a better life for yourself.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 5
    },

    { // 5
        "Narration",
        "And you're not alone either.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 6
    },

    { // 6
        "Narration",
        "You have someone who's ready to catch you if you fall, and who'll be there for your best moments too.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 7
    },

    { // 7
        "Narration",
        "You're still figuring things out though.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 8
    },

    { // 8
        "Narration",
        "Still taking it one step at a time.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 9
    },

    { // 9
        "Narration",
        "But now, when you look ahead, it doesn't feel so uncertain anymore.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 10
    },

    { // 10
        "Narration",
        "Cause all you have to do is to just…",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 11
    },

    { // 11
        "Narration",
        "…Take it slow.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 12
    },

    { // 12
        "Narration",
        "GOOD ENDING",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, -1
    }
};

static const DialogNode neutralEndingTemplate[] =
{
    { // 0
        "Narration",
        "Six months have passed since the incident.",
        EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_PLAY_SOUND,
        BG_NEUTRAL_ENDING, AVATAR_NONE, SOUND_NEUT_END,
        INSPECT_NONE,
        0, {}, 1
    },

    { // 1
        "Narration",
        "Life remains uneventful.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 2
    },

    { // 2
        "Narration",
        "You're still staying in your small apartment, and going to that same job.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 3
    },

    { // 3
        "Narration",
        "You haven't spoken to your friend in a long time either.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },

    { // 4
        "Narration",
        "But your apartment is clean now.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 5
    },

    { // 5
        "Narration",
        "You show up to work on time.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 6
    },

    { // 6
        "Narration",
        "You greet your colleagues.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 7
    },

    { // 7
        "Narration",
        "You try your best to work towards a better life for yourself.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 8
    },

    { // 8
        "Narration",
        "Even though you are alone in this journey...",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 9
    },

    { // 9
        "Narration",
        "Even though there are times where it gets really hard...",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 10
    },

    { // 10
        "Narration",
        "And it feels like you need someone there...",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 11
    },

    { // 11
        "Narration",
        "The fact that you're still moving forward is already something beautiful.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 12
    },

    { // 12
        "Narration",
        "NEUTRAL ENDING",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, -1
    }
};

static const DialogNode badEndingTemplate[] =
{
    { // 0
        "Narration",
        "It's been six months since the incident.",
        EVENT_CHANGE_BACKGROUND,
        BG_BAD_ENDING, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 1
    },

    { // 1
        "Narration",
        "The smell never really goes away.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 2
    },

    { // 2
        "Narration",
        "You stopped trying to clean it.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 3
    },

    { // 3
        "Narration",
        "You stopped trying to fix anything.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },

    { // 4
        "Narration",
        "Every day is the same.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 5
    },

    { // 5
        "Narration",
        "Wake up. Work. Come back. Sleep.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 6
    },

    { // 6
        "Narration",
        "Repeat.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 7
    },

    { // 7
        "Narration",
        "And somewhere along the way...",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 8
    },

    { // 8
        "Narration",
        "You made yourself believe that it's never going to get better.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 9
    },

    { // 9
        "Narration",
        "BAD ENDING",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, -1
    }
};

/* ========================= */

/* Copy the selected ending template into a writable buffer.
   The dialogue system mutates nodes when events trigger, so templates stay const. */
static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_ENDING_NODES) count = MAX_ENDING_NODES;
    memcpy(endingNodes, src, sizeof(DialogNode) * count);
}

/* =========================
   INIT
   ========================= */

/* Choose which ending script to play and reset the shared event state for the ending scene. */
void InitEnding(GameState endingType)
{
    waitingOnEvent = false;

    static bool resourcesLoaded = false;

    if (!resourcesLoaded)
    {
        EventsLoadResources();
        resourcesLoaded = true;
    }

    EventsInit();

    if (endingType == ENDING_GOOD)
        CopyScript(goodEndingTemplate, sizeof(goodEndingTemplate)/sizeof(DialogNode));
    else if (endingType == ENDING_SLIGHTLY_BAD)
        CopyScript(neutralEndingTemplate, sizeof(neutralEndingTemplate)/sizeof(DialogNode));
    else
        CopyScript(badEndingTemplate, sizeof(badEndingTemplate)/sizeof(DialogNode));

    DialogStart(&endingDialog, endingNodes);
}

/* =========================
   UPDATE
   ========================= */

/* Draw and advance the ending scene until the player returns to the menu. */
GameState UpdateEnding(GameState endingType)
{
    GameState requestedState;

    if (IsKeyPressed(KEY_ESCAPE) && !IsSettingsMenuOpen())
    {
        OpenPauseMenu();
        return endingType;
    }

    Texture2D *bg = EventsGetCurrentBackground();
    if (bg && bg->id)
        DrawTexture(*bg, 0, 0, WHITE);
    else
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

    {
        Texture2D *avatar = EventsGetCurrentAvatar();
        if (avatar && avatar->id)
        {
            float scale = 1.5f;
            DrawTexturePro(
                *avatar,
                (Rectangle){0,0,avatar->width,avatar->height},
                (Rectangle){
                    GetScreenWidth() - avatar->width * scale - 50,
                    GetScreenHeight() - avatar->height * scale,
                    avatar->width * scale,
                    avatar->height * scale
                },
                (Vector2){0,0},
                0,
                WHITE
            );
        }
    }

    if (IsSettingsMenuOpen())
    {
        EventsDrawOverlay();
        {
            SettingsResult settings = UpdateAndDrawSettingsMenu();
            if (settings == SETTINGS_RESULT_GO_TO_MENU)
            {
                SaveGameForState(endingType);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
        }
        return endingType;
    }

    EventsUpdate();

    if (EventsConsumeTransition(&requestedState))
        return requestedState;

    if (waitingOnEvent && !endingDialog.finished && !EventsBusy())
    {
        waitingOnEvent = false;
        DialogResume(&endingDialog);
    }

    if (!endingDialog.finished)
    {
        if (EventsShouldBlockInput())
        {
            EventsDrawOverlay();
            return endingType;
        }

        if (!waitingOnEvent)
        {
            DialogEvent ev = DialogUpdate(&endingDialog);
            if (ev != EVENT_NONE)
            {
                DialogNode *node = &endingDialog.nodes[endingDialog.index];
                EventsTrigger(ev, node->backgroundId, node->avatarId, node->soundId, node->inspectId);
                waitingOnEvent = true;
            }
        }

        if (EventsIsDialogVisible())
            DialogDraw(&endingDialog);
    }
    else
    {
        int w = GetScreenWidth();
        int h = GetScreenHeight();

        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.5f));
        DrawText("Press ENTER to restart", w/2 - 140, h/2, 20, WHITE);

        if (IsKeyPressed(KEY_ENTER))
        {
            DeleteSaveGame();
            ResetGame();
            return MENU;
        }
    }

    EventsDrawOverlay();
    return endingType;
}
