#include "level2.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"

#include <string.h>
#include <stdbool.h>

#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define MAX_LEVEL2_NODES 220

static bool level2Initialized = false;
static DialogState level2Dialog;
static DialogNode activeNodes[MAX_LEVEL2_NODES];
static bool waitingOnEvent = false;

static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_LEVEL2_NODES) count = MAX_LEVEL2_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}

static bool IsProtagonistSpeaker(const char *speaker)
{
    return (strcmp(speaker, "You") == 0) ||
           (strcmp(speaker, "Him") == 0);
}

static bool IsGirlSpeaker(const char *speaker)
{
    return strcmp(speaker, "Her") == 0;
}

static bool IsNarrationSpeaker(const char *speaker)
{
    return (strcmp(speaker, "Narration") == 0) ||
           (strcmp(speaker, "Voice") == 0) ||
           (strcmp(speaker, "Choice") == 0);
}

static int AvatarForSpeaker(const char *speaker)
{
    if (IsProtagonistSpeaker(speaker)) return AVATAR_NEUTRAL;
    if (IsGirlSpeaker(speaker)) return AVATAR_GIRL_DISAPPOINTED;
    return AVATAR_NONE;
}

/* preload avatar one real node ahead so there are no blank avatar nodes */
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
        else if (!IsNarrationSpeaker(cur->speaker))
        {
            cur->event |= EVENT_AVATAR_HIDE;
            cur->avatarId = AVATAR_NONE;
        }
    }
}

static const DialogNode level2Template[] =
{
    /* =========================
       SCENE 0 - THE ELEVATOR 2.0
       ========================= */

    { "Narration", "A pit of dread fills your stomach...",
      EVENT_FADE_IN, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 1 },

    { "Narration", "You reflect on the words that came from that strange voice.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 2 },

    { "Voice", "Remember now why it ended.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 3 },

    { "Narration", "Your thoughts race, sifting through the countless memories which could mean such a thing.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 4 },

    { "Narration", "But before you can fully process all of them, the elevator brings itself to a stop, revealing the next level.",
      EVENT_SHAKE_SCREEN | EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN,
      BG_L2_DINER, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 5 },

    /* =========================
       SCENE 1 - THE DINER, REVISITED
       ========================= */

    { "You", "Oh, I'm here again?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 6 },

    { "Narration", "Same familiar aroma, same calm lighting, same ambience.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 7 },

    { "Narration", "It all should be calm and familiar to you, but still something feels awfully wrong...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 8 },

    { "Narration", "You take your seat and as you take it, a familiar voice comes out to you.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 9 },

    { "Her", "Is this seat taken?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 10 },

    { "Narration", "You turn around and smile gently at her.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 11 },

    { "Narration", "Same polka dot dress, same blue eyes, same soft smile, but...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 12 },

    { "Narration", "For some reason this whole moment feels so wrong.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 13 },

    { "Narration", "She then takes her seat.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 14 },

    { "Narration", "Give crumpled letter.",
      EVENT_INSPECT_START, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_L2_LETTER, 0, {}, 15 },

    { "Narration", "And now she reads them out loud.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 16 },

    { "Narration", "",
      EVENT_SHOW_CARD, BG_NONE, AVATAR_NONE, SOUND_NONE,
      CARD_L2_PROPOSAL, 0, {}, 17 },

    { "Narration", "You look into her eyes, with excitement and hope.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 18 },

    { "Narration", "Looking for any signs of approval or happiness or joy...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 19 },

    { "Narration", "But instead that once simple smile melts away...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 20 },

    { "Her", "What is this?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 21 },

    { "Narration", "Her voice is quieter than you remember.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 22 },

    { "Her", "You know how much I hate this... right?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 23 },

    { "Narration", "Memories surge through your head, echoing events that happened exactly like this.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 24 },

    { "Narration", "You don't listen.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 25 },

    { "Narration", "You're pushing too hard.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 26 },

    { "Narration", "I've told you again and again, not right now.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 27 },

    { "Narration", "Please, let's just slow down.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 28 },

    /* =========================
       FIRST CHOICE
       ========================= */

    { "Choice", "How do you respond?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE,
      2,
      {
          { "Apologize again", 29 },
          { "But I thought that in the end, this is what you wanted.", 39 }
      },
      -1 },

    /* Option A */

    { "You", "I'm so so so sorry",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 30 },

    { "You", "I thought that this was the right thing to do. That this was the time.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 31 },

    { "Narration", "She looks at you, tired.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 32 },

    { "You", "I thought that it was okay to do this, but I was wrong and I'm so so sorry.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 33 },

    { "Narration", "She takes a deep breath, and speaks.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 34 },

    { "Her", "I do want to forgive you, I really do.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 35 },

    { "Her", "But this isn't the first time you've done something like this.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 36 },

    { "Her", "You can't just keep doing this over and over and over again expecting me to just 'forget' about it.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 37 },

    { "Narration", "She raises her voice, but not enough to shout at you.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 38 },

    { "Narration", "She's far too tired for that now.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 49 },

    /* Option B */

    { "You", "I thought that you wanted to enjoy the 'little things' together",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 40 },

    { "You", "Didn't you say that, that's what made life beautiful?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 41 },

    { "Narration", "She meets your eyes, tired, but unyielding.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 42 },

    { "You", "I thought you were the one who said to find your own meaning...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 43 },

    { "You", "And I've found it. It's with you.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 44 },

    { "Her", "But what about me?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 45 },

    { "Narration", "A pause.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 46 },

    { "Her", "What about what I want?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 47 },

    { "Narration", "The room feels smaller.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 48 },

    { "Her", "I didn't want to be your everything.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 50 },

    { "Her", "I just wanted to be… part of your life.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 51 },

    { "Her", "But that doesn't change the fact that being with you was beautiful.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 52 },

    { "Narration", "Her voice wavers slightly.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 53 },

    { "Her", "But you don't get it, I want to do them because I can do them, not because I have to do them.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 54 },

    { "Her", "I'm not ready for something like this, and I don't want to be pressured into something that I might regret later down the line.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 55 },

    { "Narration", "She pauses, her lips quivering.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 56 },

    { "Her", "And right now it feels like you don't care about any of that.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 57 },

    /* =========================
       SECOND CHOICE
       ========================= */

    { "Choice", "What do you say next?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE,
      2,
      {
          { "Then let's start over.", 58 },
          { "But, aren't we special?", 68 }
      },
      -1 },

    /* Option A */

    { "You", "Let's forget about everything that happened, and...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 59 },

    { "You", "Let's make it right this time, please?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 60 },

    { "Narration", "You plead with her, hoping for a different answer.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 61 },

    { "Her", "...You know deep down that you and I can't just do that.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 62 },

    { "Her", "Sometimes there is no starting over, and well I don't think there was any starting over to begin with.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 63 },

    { "Her", "You end up with what you end up, and you live with your decisions, and that's that.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 64 },

    { "Her", "But hey...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 65 },

    { "Narration", "She manages a slight but soft smile.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 66 },

    { "Her", "We had a good thing, and I'm glad we did this.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 67 },

    { "Her", "And I don't regret any of it at all.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 80 },

    /* Option B */

    { "You", "Weren't the moments we shared together special?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 69 },

    { "You", "And if so, then why not give it a second chance? Why can't we make this work in the end?",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 70 },

    { "Narration", "She pauses a little.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 71 },

    { "Her", "You aren't wrong about those moments, those were one of the most special moments in my life.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 72 },

    { "Narration", "She smiles a little.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 73 },

    { "Her", "And you were one of the most special people in my life.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 74 },

    { "Her", "But, that doesn't mean that our lives can only be special with each other.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 75 },

    { "Narration", "Her voice quivers a little.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 76 },

    { "Her", "And it also doesn't mean that you're the only special person in the world.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 77 },

    { "Narration", "She pauses.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 78 },

    { "Her", "There was a time where I felt really fulfilled by what we had here and I really enjoyed that but now…",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 79 },

    { "Her", "Now I just don't feel that anymore.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 80 },

    /* =========================
       AFTER DIALOGUE OPTIONS
       ========================= */

    { "Narration", "It feels as though your heart dropped to the diner floor.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 81 },

    { "Narration", "It almost feels like the dread can consume your entire being.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 82 },

    { "Narration", "You realize that this is how it all ended.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 83 },

    { "Choice", "Please don't do this...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE,
      1,
      {
          { "Continue", 84 }
      },
      -1 },

    { "Narration", "You plead with her and for a moment…",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 85 },

    { "Narration", "Her blue eyes waver slightly, tearing up.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 86 },

    { "Narration", "But she brings it back and does not waver.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 87 },

    { "You", "Plea-",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 88 },

    { "Narration", "But before you can plead again, the memory of the voice echoes through your head.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 89 },

    { "Voice", "But you can change your future.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 90 },

    { "Narration", "And with a heavy heart, you compose yourself.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 91 },

    { "Choice", "Try and change.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE,
      1,
      {
          { "Continue", 92 }
      },
      -1 },

    { "Narration", "You take a deep breath.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 93 },

    { "Narration", "And you say the words.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 94 },

    { "You", "...Thank you, for everything.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 95 },

    { "You", "What we shared together was special, and I won't ever forget it for the rest of my life.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 96 },

    { "You", "And..",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 97 },

    { "Narration", "Your voice wavers slightly.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 98 },

    { "You", "...I'm sorry, for everything, I know it won't change anything and you don't have to forgive me.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 99 },

    { "You", "But anyway... I'm sorry.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 100 },

    { "You", "And again, truly, thank you... for everything, I wish you all the best.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 101 },

    { "Narration", "You manage a small smile.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 102 },

    { "Narration", "She smiles back softly.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 103 },

    { "Her", "Thank you too.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 104 },

    { "Narration", "The memory fades.",
      EVENT_EYES_CLOSE | EVENT_DIALOG_HIDE | EVENT_AVATAR_HIDE | EVENT_GO_LEVEL3,
      BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, -1 }
};

static void InitLevel2State(void)
{
    waitingOnEvent = false;
    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    CopyScript(level2Template, ARRAY_COUNT(level2Template));
    ApplyAvatarPreload(activeNodes, ARRAY_COUNT(level2Template));
    DialogStart(&level2Dialog, activeNodes);

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

    EventsUpdate();

    Texture2D *bg = EventsGetCurrentBackground();
    if (bg && bg->id != 0)
        DrawTexture(*bg, 0, 0, WHITE);
    else
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

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

    if (waitingOnEvent && !level2Dialog.finished && !EventsBusy())
    {
        waitingOnEvent = false;
        DialogResume(&level2Dialog);
    }

    if (!level2Dialog.finished)
    {
        if (EventsShouldBlockInput())
            return LEVEL2;

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
    else
    {
        nextLevel = LEVEL3;
        level3Unlocked = 1;
        level2Initialized = false;
        return ELEVATOR;
    }

    EventsDrawOverlay();
    return LEVEL2;
}