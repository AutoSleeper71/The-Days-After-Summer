/* Elevator scene controller.
   Works as the story hub between levels and plays the intro / transition dialogue sequences. */

#include "elevator.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include <stddef.h>
#include <string.h>
#include "dialog_scripts.h"

// File-specific compile-time limit used to size arrays safely.
#define MAX_ELEVATOR_NODES 100

// Kept for future expansion; the elevator currently relies mostly on the shared event resource system.
static bool resourcesLoaded = false;

// dialog states
static DialogState elevatorDialog;
static DialogNode activeNodes[MAX_ELEVATOR_NODES];
static bool waitingOnSceneEvent = false;

// level states
static bool introPlaying = true;
static int introState = 0;
static float introFadeAlpha = 0.0f;
static float introEyelidTimer = 0.0f;



// Utility functions
static void CopyScript(const DialogNode *src, int count)
{
    if(count > MAX_ELEVATOR_NODES) count = MAX_ELEVATOR_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}

static void LoadElevatorIntro(void)
{
    CopyScript(ELEVATOR_INTRO_TEMPLATE, ELEVATOR_INTRO_TEMPLATE_COUNT);
}

// INIT

void InitElevator(void)
{
    waitingOnSceneEvent = false;

    static bool resourcesLoaded = false;

    EventsInit();

    introPlaying = true;
    introState = 0;
    introFadeAlpha = 0.0f;
    introEyelidTimer = 0.0f;

    LoadElevatorIntro();
    DialogStart(&elevatorDialog, activeNodes);
}

GameState UpdateElevator(void)
{
    GameState requestedState;
    Texture2D *bg;
    Texture2D *avatar;
    Vector2 shake;

    if (IsKeyPressed(KEY_ESCAPE) && !IsSettingsMenuOpen())
    {
        OpenPauseMenu();
        return ELEVATOR;
    }

    if (introPlaying && nextLevel == LEVEL1)
    {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        Texture2D *introBg = EventsGetCurrentBackground();

        if (introBg != NULL && introBg->id != 0)
            DrawTexture(*introBg, 0, 0, WHITE);
        else
            DrawRectangle(0, 0, w, h, BLACK);

        if (introState == 0)
        {
            introFadeAlpha += GetFrameTime() / 2.0f;
            if (introFadeAlpha >= 1.0f)
            {
                introFadeAlpha = 1.0f;
                introState = 1;
            }
            DrawRectangle(0, 0, w, h, Fade(BLACK, 1.0f - introFadeAlpha));
        }
        else
        {
            float lid = 0.0f;
            float dur = 1.0f;

            introEyelidTimer += GetFrameTime();

            if (introEyelidTimer < dur)
                lid = (h / 2.0f) * (introEyelidTimer / dur);
            else if (introEyelidTimer < dur * 2.0f)
                lid = (h / 2.0f) * (1.0f - ((introEyelidTimer - dur) / dur));
            else
                introPlaying = false;

            DrawRectangle(0, 0, w, (int)lid, BLACK);
            DrawRectangle(0, h - (int)lid, w, (int)lid, BLACK);
        }

        if (IsSettingsMenuOpen())
        {
            SettingsResult settings = UpdateAndDrawSettingsMenu();
            if (settings == SETTINGS_RESULT_GO_TO_MENU)
            {
                SaveGameForState(ELEVATOR);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
        }

        return ELEVATOR;
    }

    bg = EventsGetCurrentBackground();
    shake = EventsGetShakeOffset();

    if (bg != NULL && bg->id != 0)
        DrawTexture(*bg, (int)shake.x, (int)shake.y, WHITE);
    else
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

    avatar = EventsGetCurrentAvatar();
    if (avatar != NULL && avatar->id != 0)
    {
        float scale = 1.5f;
        float cw = avatar->width * scale;
        float ch = avatar->height * scale;
        float px = GetScreenWidth() - cw - 50;
        float py = GetScreenHeight() - ch;

        DrawTexturePro(
            *avatar,
            (Rectangle){0, 0, (float)avatar->width, (float)avatar->height},
            (Rectangle){px, py, cw, ch},
            (Vector2){0, 0},
            0,
            WHITE
        );
    }

    if (IsSettingsMenuOpen())
    {
        if (EventsIsDialogVisible())
            DialogDraw(&elevatorDialog);

        EventsDrawOverlay();

        {
            SettingsResult settings = UpdateAndDrawSettingsMenu();
            if (settings == SETTINGS_RESULT_GO_TO_MENU)
            {
                SaveGameForState(ELEVATOR);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
        }
        return ELEVATOR;
    }

    EventsUpdate();

    if (EventsConsumeTransition(&requestedState))
        return requestedState;

    if (waitingOnSceneEvent && !EventsBusy())
    {
        waitingOnSceneEvent = false;
        DialogResume(&elevatorDialog);
    }

    if (!elevatorDialog.finished)
    {
        if (EventsShouldBlockInput())
        {
            EventsDrawOverlay();
            return ELEVATOR;
        }

        {
            DialogEvent ev = DialogUpdate(&elevatorDialog);
            if (ev != EVENT_NONE)
            {
                DialogNode *node = &elevatorDialog.nodes[elevatorDialog.index];
                EventsTrigger(ev, node->backgroundId, node->avatarId, node->soundId, node->inspectId);

                if (EventsBusy())
                    waitingOnSceneEvent = true;
                else
                    DialogResume(&elevatorDialog);
            }
        }

        if (EventsIsDialogVisible())
            DialogDraw(&elevatorDialog);
    }

    EventsDrawOverlay();
    return ELEVATOR;
}
