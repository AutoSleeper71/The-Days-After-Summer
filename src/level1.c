/* Level 1 scene logic.
   Builds the dialogue script for the first memory and runs its intro, events, and transitions. */

#include "level1.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"

#include <string.h>
#include <stdbool.h>

// Helper macro used to get the number of elements in a fixed-size array.
#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
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

/* Level 1 narrative script.
   The comments inside the array split the level into story scenes. */
static const DialogNode level1Template[] =
{
    /* =========================
       SCENE 0 – ELEVATOR
       ========================= */

    { "Narration", "You take a moment to catch your breath, slowly coming back to your senses.", EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY, INSPECT_NONE, 0, {}, 1 },
    { "You", "God... That was really close...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 2 },
    { "Narration", "But before you can fully recover from that experience,", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 3 },
    { "Narration", "The elevator suddenly groans, and comes to a stop.", EVENT_SHAKE_SCREEN | EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_STOP_SOUNDS, BG_L1_DINER, AVATAR_NONE, SOUND_ELEVATOR_SCARY, INSPECT_NONE, 0, {}, 4 },

    /* =========================
       SCENE 1 – DINER
       ========================= */

    { "Narration", "Warm light floods the cold elevator, the familiar aroma of food and vanilla fill the air, replacing the staleness of the elevator with the smell of a well-known comfort.", EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_TALKING, INSPECT_NONE, 0, {}, 5 },
    { "Narration", "Friends laugh. Couples chat. Families eat.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 6 },
    { "You", "What is this place?", EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_ELEVATOR, INSPECT_NONE, 0, {}, 7 },
    { "Narration", "Welcome to your memories, Daniel.", EVENT_AVATAR_HIDE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 8 },
    { "Narration", "A voice inserts itself into the crevices of your brain, as if it's your own very thoughts talking right to you.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 9 },
    { "Narration", "You spin in confusion. There is no one there.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 10 },
    { "You", "...Okay, that's not unsettling at all.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 11 },
    { "You", "Hello? Anyone there? If this is a prank, it's not very funny.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 12 },
    { "Narration", "The sounds of the diner dampen, as if it's lowering its voice for someone else to speak...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 13 },
    { "You", "Who are you?! What is this place…?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 14 },
    { "Narration", "This is something you buried deep within. A memory you refused to remember.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 15 },
    { "Narration", "You will face the memories you have run away from.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 16 },
    { "Narration", "You cannot change your past.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 17 },
    { "Narration", "But you can change your future.", EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_TALKING, INSPECT_NONE, 0, {}, 18 },
    { "Narration", "Suddenly, you feel an eerie presence slip through the base of your skull.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 19 },
    { "Narration", "The sounds of the diner rush back, covering you in a blanket of warmth and comfort.", EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_L1_DINER_BOOTH, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 20 },
    { "Narration", "Suddenly a wave of realization crashes over you, unlocking memories buried deep below.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 21 },
    { "Narration", "This is where I met her.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 22 },
    { "You", "I remember that day.", EVENT_SHOW_CARD, BG_NONE, AVATAR_NONE, SOUND_NONE, CARD_L1_DAY, 0, {}, 23 },
    { "You", "My entire life was so draining.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 24 },
    { "You", "All of everything, all at once.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 25 },
    { "You", "Everywhere, day-in, day-out.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 26 },
    { "You", "I thought I needed to do this, that these were for attaining my aspirations and my ambitions.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 27 },
    { "You", "You know with all of my smartness, I thought that I had everything figured out.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 28 },
    { "You", "I really didn't.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 29 },
    { "Narration", "Then, a voice behind you.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 30 },
    { "Her", "Is this seat taken?", EVENT_PLAY_SOUND, BG_NONE, AVATAR_GIRL_HAPPY, SOUND_ELEVATOR_DING, INSPECT_NONE, 0, {}, 31 },
    { "Narration", "You freeze, and slowly turn around.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 32 },
    { "Narration", "She stands there. Simple. Colorful. Beautiful.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 33 },
    { "Narration", "A polka-dot dress. Bright blue eyes. And a soft, effortless smile.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 34 },

    { "You", "...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 2,
      {
          {"No, not at all.", 35},
          {"Only if you don't mind terrible company.", 38}
      }, -1 },

    /* first choice branch */
    { "Her", "Good.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 36 },
    { "Her", "Because I hate sitting alone.", EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_L1_DINER_BOOTH, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 37 },
    { "You", "Good to know I passed the test.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 40 },

    /* second choice branch */
    { "Her", "Well fortunately for me, it seems like you're juuuust not-terrible enough for me, which is good enough in my book!", EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_L1_DINER_BOOTH, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 39 },
    { "You", "Wow. Setting the bar high, I see.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 40 },

    /* merge */
    { "Narration", "She softly chuckles to herself and takes her seat on the opposite end.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 41 },
    { "Her", "Well! What brings such a fine company here?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 42 },
    { "Narration", "She makes a sly wink.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 43 },

    { "You", "...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 2,
      {
          {"Well, work, I guess.", 44},
          {"I guess it's all kind of stressful you know?", 52}
      }, -1 },

    /* response A */
    { "Narration", "(........)", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 45 },
    { "Narration", "A long silence fills the conversation….", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 46 },
    { "Her", "Are you seriously just going to say that? Could ya' add any more details other than just, 'work'?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 47 },
    { "Narration", "She raises her eyebrow and side eyes you.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 48 },
    { "You", "I mean, you know, stranger danger? I can't just be spilling my life right in front of a stranger. Who knows how you might use this information against me? You know?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 49 },
    { "Narration", "She makes a slight smirk.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 50 },
    { "Her", "Well, at least this stranger has the kindness and the compassion to talk to someone like you, thank you very much!", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 51 },
    { "Narration", "We both chuckle slightly, and the stress of everything kind of falls away.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 58 },

    /* response B */
    { "You", "Between balancing work, exercise, eating right, friends, family, etcetera, etcetera, everything kind of gets overwhelming very quickly.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 53 },
    { "Her", "Oh my god that's quite a lot huh? No wonder it looks like you're doing the latest rendition of an underpaid salaryman.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 54 },
    { "Narration", "She chuckles to herself slightly, letting out a comforting grin.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 55 },
    { "You", "Hey! C'mon, low blow much? It's one thing to randomly approach a stranger but it's another to insult how they look on sight?!", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 56 },
    { "Narration", "She bursts out laughing, and it illuminates the whole room.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 57 },
    { "Narration", "And you can't help but let out the biggest smile even after being insulted directly to your face.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 58 },

    /* after both responses */
    { "Narration", "And just like that—", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 59 },
    { "Narration", "something shifts.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 60 },
    { "Narration", "You feel like in that one moment, with her smile that felt like it illuminated your whole world, that everything stood still just for her, and all the worries, burdens, and responsibilities all seemed to just…", EVENT_SHOW_CARD, BG_NONE, AVATAR_NONE, SOUND_NONE, CARD_L1_MELT, 0, {}, 61 },
    { "Her", "Well? Are you gonna order or?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 62 },
    { "Narration", "You suddenly are reminded why you came here in the first place,", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 63 },
    { "Narration", "to grab some food.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 64 },
    { "You", "Ah! Right then, let's see here...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 65 },
    { "Narration", "You glance at several options.", EVENT_INSPECT_START | EVENT_STOP_SOUNDS, BG_NONE, AVATAR_NONE, SOUND_TALKING, INSPECT_L1_DINER_MENU, 0, {}, 66 },

    { "Narration", "Suddenly, the whole diner becomes quiet.", EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_PLAY_SOUND | EVENT_AVATAR_HIDE, BG_L1_DINER_EMPTY, AVATAR_NONE, SOUND_ELEVATOR_SCARY, INSPECT_NONE, 0, {}, 67 },
    { "Narration", "The lights are turned off.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 68 },
    { "Narration", "The chairs are on the tables.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 69 },
    { "Narration", "She is no longer there anymore.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 70 },
    { "Narration", "Beginnings are easy to remember.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 71 },
    { "Narration", "Endings are harder.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 72 },
    { "Narration", "The diner lights flicker.", EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_PLAY_SOUND, BG_L1_PARK, AVATAR_NONE, SOUND_PARK, INSPECT_NONE, 0, {}, 73 },

    /* =========================
       SCENE 2 – PARK
       ========================= */

    { "Narration", "You step onto a modest, grassy hill overlooking the great city beyond.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 74 },
    { "Narration", "The afternoon sky glows in shades of gold and pink, layering the world in a blanket of comforting warmth, and enveloping you in its warm light.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 75 },
    { "Narration", "Songbirds chirp softly, and the faint sounds of cars, traffic, and urban life are drowned out by the birds’ beautiful song.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 76 },
    { "You", "...Our favorite place.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 77 },
    { "Narration", "Footsteps approach.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 78 },
    { "Narration", "Then, she appears.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 79 },
    { "Narration", "She carries a picnic basket, smiling the same effortless smile.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 80 },
    { "Her", "So...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 81 },
    { "Her", "Are you going to join me?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 82 },
    { "You", "...I was waiting for a formal invitation.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 83 },
    { "Her", "Oh, my bad.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 84 },
    { "Narration", "She clears her throat dramatically….", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 85 },
    { "Narration", "...and lets out the most horrendous British accent that could ever exist.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 86 },
    { "Her", "Dear sir, would you kindly join me for this absolutely gourmet dining experience?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 87 },
    { "Narration", "You laugh and cringe all at the same time, feeling the entirety of the second-hand embarrassment from that gesture.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 88 },
    { "You", "Now that sounds convincing.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 89 },
    { "Narration", "You walk toward her, gently taking her hand and sit together on the bench.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 90 },
    { "Narration", "It feels as though there is nothing else outside of this bench,", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 91 },
    { "Narration", "and you,", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 92 },
    { "Narration", "and her.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 93 },
    { "Her", "Look, I brought some things! Come and grab some!", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 94 },
    { "Narration", "She opens the picnic basket.", EVENT_INSPECT_START, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_L1_PARK_BASKET, 0, {}, 95 },

    { "Her", "You know why I love sunsets?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 96 },
    { "You", "Because you're secretly a hopeless romantic?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 97 },
    { "Narration", "She laughs softly.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 98 },
    { "Her", "No. Far from it actually.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 99 },
    { "Narration", "She looks towards the horizon.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 100 },
    { "Her", "Because they remind me that everything, even endings, can be beautiful.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 101 },
    { "Narration", "She gestures toward the sky.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 102 },
    { "Her", "We get so caught up in attaching meaning to everything...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 103 },
    { "Her", "To strive towards something...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 104 },
    { "Her", "And once we 'have it', we go on to the next thing...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 105 },
    { "Her", "And the next thing...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 106 },
    { "Narration", "A pause.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 107 },

    { "Her", "Don't you think it's all... a bit useless sometimes?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 2,
      {
          {"Yeah, it kinda is, isn't that why it kinda sucks?", 108},
          {"Not at all, how could that be?", 114}
      }, -1 },

    /* philosophy response A */
    { "Her", "I wouldn't say so at all, It means that you are free to do anything you want.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 109 },
    { "Narration", "She reaches a hand towards the twilight sky, her eyes sparkling as bright as the stars above.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 110 },
    { "Her", "It means that you are free to cry and laugh, to fail and succeed, to fall down and get up, over and over and over again.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 111 },
    { "Her", "It means that you are free to find your own meaning, your own destiny.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 112 },
    { "Her", "And that nothing, nothing, can ever take that away from you.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 113 },
    { "Her", "And I find that truly beautiful.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 123 },

    /* philosophy response B */
    { "Her", "Sure! And if you really believe so, all the more power to you!", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 115 },
    { "Her", "That means that those things are your meaning.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 116 },
    { "Her", "My meaning is different though, and I don't think it's any less meaningful than your hopes and dreams.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 117 },
    { "Narration", "She gazes out at the horizon, her eyes glowing warmer than the evening sun.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 118 },
    { "Her", "... I like the little things,", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 119 },
    { "Her", "a well-made cheese sandwich, with a bottle of good lemonade, and just...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 120 },
    { "Her", "Just watching the sun go down with someone special.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 121 },
    { "Narration", "She turns towards you, with a simple yet heartwarming smile.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 122 },
    { "Her", "I think that's what makes life beautiful.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 123 },

    /* merge */
    { "Her", "Ah well, that's just me yapping though!", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 124 },
    { "Narration", "You also gaze up at the stars.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 125 },
    { "Narration", "They truly are beautiful this evening.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 126 },
    { "Narration", "You look back down and smirk slightly.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 127 },
    { "You", "Mhmm, mhmmm thank you professor for this wonderful lecture, when can we hear from you again madam?", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 128 },
    { "Narration", "She makes a slight grin.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 129 },
    { "Her", "Oh shut up! In that case, I'll be failing you right here, right now just for your disrespect!", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 130 },
    { "Narration", "You both laugh into the evening sky.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 131 },
    { "Her", "Anyways, to summarize, sometimes...", EVENT_NONE | EVENT_STOP_SOUNDS, BG_NONE, AVATAR_NONE, SOUND_PARK, INSPECT_NONE, 0, {}, 132 },
    { "Narration", "I feel like it's best to just...", EVENT_SHOW_CARD | EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY, CARD_L1_SLOW, 0, {}, 133 },

    { "Narration", "She is gone.", EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN | EVENT_AVATAR_HIDE, BG_L1_PARK_EMPTY, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 134 },
    { "Narration", "Silence.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 135 },
    { "Narration", "The sunset fades away.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 136 },
    { "Narration", "The only thing you have now is that same crumpled letter in your hand.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 137 },
    { "Narration", "And it all clicks in your head.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 138 },
    { "Narration", "And you shudder at the realization of what it means.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 139 },
    { "You", "It, just, can't be...", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 140 },
    { "Narration", "A wave of thoughts and memories come crashing down you.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 141 },
    { "Narration", "It almost overwhelming you with its information.", EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 142 },
    { "Narration", "But now you know what happened.", EVENT_SHOW_CARD, BG_NONE, AVATAR_NONE, SOUND_NONE, CARD_L1_LETTER, 0, {}, 143 },
    { "Narration", "Suddenly you feel that same presence, bury itself in your head.", EVENT_AVATAR_HIDE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 144 },
    { "Narration", "Now you remember how the story ends.", EVENT_NONE | EVENT_STOP_SOUNDS, BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY, INSPECT_NONE, 0, {}, 145 },
    { "Narration", "Remember now why it ended.", EVENT_EYES_CLOSE | EVENT_DIALOG_HIDE | EVENT_AVATAR_HIDE | EVENT_GO_LEVEL2, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, -1 }
};

/* Build runtime data for level 1 and apply a short manual fade before dialogue begins. */
static void InitLevel1State(void)
{
    int count = ARRAY_COUNT(level1Template);

    waitingOnEvent = false;
    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    CopyScript(level1Template, count);
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
