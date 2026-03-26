#include "elevator.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include <stddef.h>
#include <string.h>

#define MAX_ELEVATOR_NODES 100
#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

static bool resourcesLoaded = false;

static DialogState elevatorDialog;
static DialogNode activeNodes[MAX_ELEVATOR_NODES];
static bool waitingOnSceneEvent = false;

static bool introPlaying = true;
static int introState = 0;
static float introFadeAlpha = 0.0f;
static float introEyelidTimer = 0.0f;

static const DialogNode introTemplate[] =
{
    // 0
    {"???", "...Where am I?",
     EVENT_AVATAR_SHOW | EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND,
     BG_INSIDE, AVATAR_NEUTRAL, SOUND_ELEVATOR_SCARY,
     INSPECT_NONE, 0, {}, 1},

    // 1
    {"Daniel", "Ah right, I'm clocking into work today, and I'm running a little late too..",
     EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 2},

    // 2
    {"Narration", "Slowly you come to your senses, carefully observing the environment around you.",
     EVENT_AVATAR_SHOW,
     BG_NONE, AVATAR_CONFUSED, SOUND_NONE,
     INSPECT_NONE, 0, {}, 3},

    // 3
    {"Narration", "The elevator feels wrong. Same space, same lights...",
     EVENT_AVATAR_SHOW | EVENT_SHAKE_SCREEN,
     BG_NONE, AVATAR_NEUTRAL, SOUND_NONE,
     INSPECT_NONE, 0, {}, 4},

    // 4
    {"Narration", "Just...",
     EVENT_EYES_CLOSE | EVENT_STOP_SOUNDS,
     BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY,
     INSPECT_NONE, 0, {}, 5},

    // 5
    {"Narration", "No buttons.",
     EVENT_EYES_OPEN,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 6},

    // 6
    {"Daniel", "Huh... no buttons? Where am I?",
     EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND | EVENT_FADE_IN,
     BG_ROOM, AVATAR_NONE, SOUND_ELEVATOR,
     INSPECT_NONE, 0, {}, 7},

    // 7
    {"Daniel", "Isn't this my room? God it smells here",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 8},

    // 8
    {"Narration", "The smell is overwhelming.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 9},

    // 9 (CHOICE)
    {"Daniel", "What should I do now?",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     2,
     {
        {"Go to bathroom", 10},
        {"Stay here", 11}
     },
     -1},

    // 10 → bathroom
    {"Daniel", "I should wash up a little.",
     EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND | EVENT_FADE_IN | EVENT_STOP_SOUNDS,
     BG_BATHROOM, AVATAR_NONE, SOUND_LIGHT,
     INSPECT_NONE, 0, {}, 12},

    // 11 → forced bathroom
    {"Narration", "You try to ignore the smell, but it only gets worse.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 10},

    // 12
    {"Daniel", "Ah... crap",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS,
     BG_BATHROOM, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 13},

    // 13 (BATHROOM HUB)
    {"Narration", "What should I do?",
     EVENT_PLAY_SOUND,
     BG_NONE, AVATAR_NONE, SOUND_LIGHT,
     INSPECT_NONE,
     2,
     {
        {"Check bathroom", 14},
        {"Go back", 15}
     },
     -1},

    // 14 (INSPECT)
    {"Narration", "You take a closer look around.",
     EVENT_INSPECT_START,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_BATHROOM_ITEMS,
     0, {}, 13},

    // 15 (EXIT)
    {"Daniel", "I am so tired",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS | EVENT_AVATAR_HIDE | EVENT_FADE_IN,
     BG_LOBBY, AVATAR_NONE, SOUND_LIGHT,
     INSPECT_NONE, 0, {}, 16},

    // 16
    {"Daniel", "I'm here now?",
     EVENT_PLAY_SOUND,
     BG_NONE, AVATAR_NONE, SOUND_TALKING,
     INSPECT_NONE, 0, {}, 17},

    // 17
    {"Narration", "You quickly scan the room.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 18},

    // 18
    {"Colleague A", "Hey good morning!",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 19},

    // 19 (CHOICE)
    {"Colleague B", "Top of the morning!",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     2,
     {
        {"Ignore them", 20},
        {"Greet them", 21}
     },
     -1},

    // 20
    {"Daniel", "No, thanks.",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS | EVENT_AVATAR_SHOW,
     BG_INSIDE, AVATAR_NEUTRAL, SOUND_TALKING,
     INSPECT_NONE, 0, {}, 22},

    // 21
    {"Narration", "Your voice fails you.",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS | EVENT_AVATAR_SHOW | EVENT_FADE_IN,
     BG_INSIDE, AVATAR_SAD, SOUND_TALKING,
     INSPECT_NONE, 0, {}, 22},

    // 22
    {"Narration", "You rush back into the elevator.",
     EVENT_PLAY_SOUND,
     BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY,
     INSPECT_NONE, 0, {}, 23},

    // 23
    {"Narration", "The elevator waits.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     2,
     {
        {"Close door", 24},
        {"Do nothing", 23}
     },
     -1},

    // 24
    {"Daniel", "Maybe I should try harder...",
     EVENT_PLAY_SOUND,
     BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY,
     INSPECT_NONE, 0, {}, 25},

    // 25
    {"Daniel", "Maybe more...",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     1,
     {
        {"Close door", 26}
     },
     -1},

    // 26
    {"Daniel", "Maybe even more...",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     1,
     {
        {"Close door", 27}
     },
     -1},

    // 27
    {"Daniel", "Please...",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     1,
     {
        {"Close door", 28}
     },
     -1},

    // 28 (FINAL)
    {"Narration", "The elevator door finally closes...",
     EVENT_FADE_OUT | EVENT_GO_LEVEL1 | EVENT_STOP_SOUNDS,
     BG_NONE, AVATAR_NONE, SOUND_ELEVATOR,
     INSPECT_NONE, 0, {}, -1}
};

static const DialogNode toLevel2Template[] =
{
    {
        "Daniel",
        "The doors slide shut behind me.",
        EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_AVATAR_SHOW,
        BG_INSIDE, AVATAR_NEUTRAL, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 1
    },
    {
        "Daniel",
        "That room felt too familiar.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        2,
        {
            {"Take a breath", 2},
            {"Keep staring at the floor", 3}
        },
        -1
    },
    {
        "Daniel",
        "I should keep moving.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },
    {
        "Daniel",
        "If I stop now, I will think too much.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },
    {
        "Daniel",
        "The elevator groans and starts again.",
        EVENT_PLAY_SOUND | EVENT_FADE_OUT | EVENT_GO_LEVEL2,
        BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_DING,
        INSPECT_NONE,
        0, {}, -1
    }
};

static const DialogNode toLevel3Template[] =
{
    {
        "Narration",
        "You told yourself you're okay now.",
        EVENT_AVATAR_SHOW,
        BG_NONE, AVATAR_NEUTRAL, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 1
    },
    {
        "Narration",
        "You let her go.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 2
    },
    {
        "Narration",
        "You gave her your blessing.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 3
    },
    {
        "Narration",
        "But it clings on, like a curse.",
        EVENT_PLAY_SOUND,
        BG_NONE, AVATAR_NONE, SOUND_RUMBLE,
        INSPECT_NONE,
        0, {}, 4
    },
    {
        "Narration",
        "A deep pain grows and festers.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 5
    },
    {
        "Narration",
        "A pain you have never been able to come to terms with before.",
        EVENT_STOP_SOUNDS,
        BG_NONE, AVATAR_NONE, SOUND_RUMBLE,
        INSPECT_NONE,
        0, {}, 6
    },
    {
        "Narration",
        "Suddenly, you feel your phone buzz.",
        EVENT_PLAY_SOUND,
        BG_NONE, AVATAR_NONE, SOUND_TALKING,
        INSPECT_NONE,
        0, {}, 7
    },
    {
        "Him",
        "...Hello?",
        EVENT_STOP_SOUNDS,
        BG_NONE, AVATAR_NONE, SOUND_TALKING,
        INSPECT_NONE,
        0, {}, 8
    },
    {
        "Narration",
        "A familiar and friendly voice answers.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 9
    },
    {
        "Friend",
        "Hellooo!",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 10
    },
    {
        "Friend",
        "Ever since the break up we haven't been able to contact you for so long, are you okay?",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 11
    },
    {
        "Him",
        "(......)",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 12
    },
    {
        "Narration",
        "You say nothing.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 13
    },
    {
        "Narration",
        "The silence stretches.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 14
    },
    {
        "Narration",
        "It feels unbearable from the other side.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 15
    },
    {
        "Narration",
        "...",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 16
    },
    {
        "Him",
        "...Hello?",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 17
    },
    {
        "Him",
        "I'm going to see her again.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 18
    },
    {
        "Narration",
        "Your heart drops, a deep anxiety rising within you.",
        EVENT_PLAY_SOUND | EVENT_SHAKE_SCREEN,
        BG_NONE, AVATAR_NONE, SOUND_RUMBLE,
        INSPECT_NONE,
        0, {}, 19
    },
    {
        "Narration",
        "Despite leaving you in the dark for so long,",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 20
    },
    {
        "Narration",
        "Maybe it is time to help?",
        EVENT_PLAY_SOUND | EVENT_FADE_OUT | EVENT_GO_LEVEL3,
        BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_DING,
        INSPECT_NONE,
        0, {}, -1
    }
};

static const DialogNode toLevel4Template[] =
{
    {
        "Daniel",
        "It is quieter now.",
        EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_AVATAR_SHOW,
        BG_INSIDE, AVATAR_NEUTRAL, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 1
    },
    {
        "Daniel",
        "Too quiet.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        2,
        {
            {"Listen to the silence", 2},
            {"Look away", 3}
        },
        -1
    },
    {
        "Daniel",
        "It feels heavier every time the doors close.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },
    {
        "Daniel",
        "Maybe there is nothing left to say.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 4
    },
    {
        "Daniel",
        "One more floor.",
        EVENT_PLAY_SOUND | EVENT_FADE_OUT | EVENT_GO_LEVEL4,
        BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_DING,
        INSPECT_NONE,
        0, {}, -1
    }
};

static const DialogNode endingTemplate[] =
{
    {
        "Daniel",
        "This is the last ride.",
        EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_AVATAR_SHOW,
        BG_INSIDE, AVATAR_NEUTRAL, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 1
    },
    {
        "Daniel",
        "Whatever waits beyond these doors... I earned it.",
        EVENT_NONE,
        BG_NONE, AVATAR_NONE, SOUND_NONE,
        INSPECT_NONE,
        0, {}, 2
    },
    {
        "Daniel",
        "...Let's see how this ends.",
        EVENT_PLAY_SOUND | EVENT_FADE_OUT | EVENT_GO_ENDING,
        BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_DING,
        INSPECT_NONE,
        0, {}, -1
    }
};

static void CopyScript(const DialogNode *src, int count)
{
    if(count > MAX_ELEVATOR_NODES) count = MAX_ELEVATOR_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}

static void LoadScriptForNextLevel(void)
{
    if(nextLevel == LEVEL1)
        CopyScript(introTemplate, ARRAY_COUNT(introTemplate));
    else if(nextLevel == LEVEL2)
        CopyScript(toLevel2Template, ARRAY_COUNT(toLevel2Template));
    else if(nextLevel == LEVEL3)
        CopyScript(toLevel3Template, ARRAY_COUNT(toLevel3Template));
    else if(nextLevel == LEVEL4)
        CopyScript(toLevel4Template, ARRAY_COUNT(toLevel4Template));
    else
        CopyScript(endingTemplate, ARRAY_COUNT(endingTemplate));
}

void InitElevator(void)
{
    waitingOnSceneEvent = false;

    static bool resourcesLoaded = false;

    EventsInit();

    introPlaying = (nextLevel == LEVEL1);
    introState = 0;
    introFadeAlpha = 0.0f;
    introEyelidTimer = 0.0f;

    LoadScriptForNextLevel();
    DialogStart(&elevatorDialog, activeNodes);
}

GameState UpdateElevator(void)
{
    GameState requestedState;
    Texture2D *bg;
    Texture2D *avatar;
    Vector2 shake;

    if(introPlaying && nextLevel == LEVEL1)
    {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        Texture2D *introBg = EventsGetCurrentBackground();

        if(introBg != NULL)
            DrawTexture(*introBg, 0, 0, WHITE);

        if(introState == 0)
        {
            introFadeAlpha += GetFrameTime() / 2.0f;

            if(introFadeAlpha >= 1.0f)
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

            if(introEyelidTimer < dur)
                lid = (h / 2.0f) * (introEyelidTimer / dur);
            else if(introEyelidTimer < dur * 2.0f)
                lid = (h / 2.0f) * (1.0f - ((introEyelidTimer - dur) / dur));
            else
                introPlaying = false;

            DrawRectangle(0, 0, w, (int)lid, BLACK);
            DrawRectangle(0, h - (int)lid, w, (int)lid, BLACK);
        }

        return ELEVATOR;
    }

    EventsUpdate();

    /* ALWAYS check transition first */
    if(EventsConsumeTransition(&requestedState))
        return requestedState;

    /* THEN handle waiting */
    if(waitingOnSceneEvent && !EventsBusy())
    {
        waitingOnSceneEvent = false;
        DialogResume(&elevatorDialog);
    }
    shake = EventsGetShakeOffset();
    bg = EventsGetCurrentBackground();

if(bg != NULL && bg->id != 0)
    DrawTexture(*bg, (int)shake.x, (int)shake.y, WHITE);
else
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

    avatar = EventsGetCurrentAvatar();

    if(avatar != NULL)
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

    if(!elevatorDialog.finished)
    {
        if(EventsShouldBlockInput())
            return ELEVATOR;

        DialogEvent ev = DialogUpdate(&elevatorDialog);

        if(ev != EVENT_NONE)
        {
            DialogNode *node = &elevatorDialog.nodes[elevatorDialog.index];

            EventsTrigger(ev,
                node->backgroundId,
                node->avatarId,
                node->soundId,
                node->inspectId);

            if(EventsBusy())
                waitingOnSceneEvent = true;
            else
                DialogResume(&elevatorDialog);
        }

        if(EventsIsDialogVisible())
            DialogDraw(&elevatorDialog);
    }

    EventsDrawOverlay();
    return ELEVATOR;
}

void UnloadElevator(void)
{
    // Resources are now owned by events.c
    // Nothing to unload here
}