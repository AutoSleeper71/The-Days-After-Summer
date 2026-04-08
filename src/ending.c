/* Ending controller.
   Selects one of several ending scripts and plays it through the shared dialogue/event system. */

#include "ending.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include <string.h>
#include "dialog_scripts.h"

// File-specific compile-time limit used to size arrays safely.
#define MAX_ENDING_NODES 64

static DialogState endingDialog;
static DialogNode endingNodes[MAX_ENDING_NODES];

/* controls flow */
static bool waitingOnEvent = false;



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
    CopyScript(GOOD_ENDING_TEMPLATE, GOOD_ENDING_TEMPLATE_COUNT);
    else if (endingType == ENDING_SLIGHTLY_BAD)
    CopyScript(NEUTRAL_ENDING_TEMPLATE, NEUTRAL_ENDING_TEMPLATE_COUNT);
    else
    CopyScript(BAD_ENDING_TEMPLATE, BAD_ENDING_TEMPLATE_COUNT);

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
