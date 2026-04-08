#include "dialog_scripts.h"
#include "events.h"

#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

const DialogNode ELEVATOR_INTRO_TEMPLATE[] =
{
    // 0
    {"???", "...Where am I?",
     EVENT_AVATAR_SHOW | EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND,
     BG_INSIDE, AVATAR_NEUTRAL, SOUND_ELEVATOR_SCARY,
     INSPECT_NONE, 0, {}, 1},

    // 1
    {"You", "Ah right, I'm clocking into work today, and I'm running a little late too..",
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
    {"You", "Huh... no buttons? Where am I?",
     EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND | EVENT_FADE_IN,
     BG_ROOM, AVATAR_NONE, SOUND_ELEVATOR,
     INSPECT_NONE, 0, {}, 7},

    // 7
    {"You", "Isn't this my room? God it smells here",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 8},

    // 8
    {"Narration", "The smell is overwhelming.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 9},

    // 9 (CHOICE)
    {"You", "What should I do now?",
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
    {"You", "I should wash up a little.",
     EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND | EVENT_FADE_IN | EVENT_STOP_SOUNDS,
     BG_BATHROOM, AVATAR_NONE, SOUND_LIGHT,
     INSPECT_NONE, 0, {}, 12},

    // 11 → forced bathroom
    {"Narration", "You try to ignore the smell, but it only gets worse.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 10},

    // 12
    {"Narration", "You walk into the bathroom, almost being blinded by the bright fluorescent lights.",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS,
     BG_BATHROOM, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 13},

    // 13 (BATHROOM HUB)
    {"Narration", "Wash up",
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
    {"Narration", "Your tired eyes finally close.",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS | EVENT_AVATAR_HIDE | EVENT_FADE_IN,
     BG_LOBBY, AVATAR_NONE, SOUND_LIGHT,
     INSPECT_NONE, 0, {}, 16},

    // 16
    {"You", "I'm here now?",
     EVENT_PLAY_SOUND,
     BG_NONE, AVATAR_NONE, SOUND_TALKING,
     INSPECT_NONE, 0, {}, 17},

    // 17
    {"Narration", "You quickly scan the room, and see your old colleagues greeting you.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 18},

    // 18
    {"Colleague A", "Hey, good morning!",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 19},

    // 19 (CHOICE)
    {"Colleague B", "Top of the morning to ya!",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     2,
     {
        {"Ignore them", 20},
        {"Greet them back", 21}
     },
     -1},

        // 20
    {"You", "No thanks.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 22},

    // 21
    {"Narration", "Your throat tightens, sweat trickles down your forehead, and your thoughts spiral out of control. You try to speak and a weak sound escapes... and is swallowed by silence.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 22},

    // 22
    {"Narration", "You then rush towards the elevator, walking as fast as you can.",
     EVENT_CHANGE_BACKGROUND | EVENT_STOP_SOUNDS | EVENT_AVATAR_HIDE | EVENT_FADE_IN,
     BG_LOBBY_BACK, AVATAR_NONE, SOUND_TALKING,
     INSPECT_NONE, 0, {}, 23},

    // 23
    {"Colleague A", "It really shook him huh.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE, 0, {}, 24},

    // 24
    {"Colleague B", "Yeah it must have, he's stopped talking to anyone for a long time now, I wonder if he'll get over her.",
    EVENT_CHANGE_BACKGROUND | EVENT_AVATAR_SHOW | EVENT_FADE_IN,
     BG_INSIDE, AVATAR_NEUTRAL, SOUND_NONE, INSPECT_NONE, 0, {}, 25},

    // 25
    {"Narration", "The elevator waits.",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     2,
     {
        {"Close door", 26},
        {"Do nothing", 25}
     },
     -1},

    // 26
    {"You", "Maybe I should try harder...",
     EVENT_PLAY_SOUND,
     BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY,
     INSPECT_NONE, 0, {}, 27},

    // 27
    {"You", "Maybe more...",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     1,
     {
        {"Close door", 28}
     },
     -1},

    // 28
    {"You", "Maybe even more...",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     1,
     {
        {"Close door", 29}
     },
     -1},

    // 29
    {"You", "Please...",
     EVENT_NONE,
     BG_NONE, AVATAR_NONE, SOUND_NONE,
     INSPECT_NONE,
     1,
     {
        {"Close door", 30}
     },
     -1},

    // 30 (FINAL)
    {"Narration", "The elevator door finally closes...",
     EVENT_FADE_OUT | EVENT_GO_LEVEL1 | EVENT_STOP_SOUNDS,
     BG_NONE, AVATAR_NONE, SOUND_ELEVATOR,
     INSPECT_NONE, 0, {}, -1}
};
const int ELEVATOR_INTRO_TEMPLATE_COUNT = ARRAY_COUNT(ELEVATOR_INTRO_TEMPLATE);

const DialogNode LEVEL1_TEMPLATE[] =
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
const int LEVEL1_TEMPLATE_COUNT = ARRAY_COUNT(LEVEL1_TEMPLATE);

const DialogNode LEVEL2_TEMPLATE[] =
{
    { "Narration", "A pit of dread fills your stomach...",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
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
      EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_TALKING,
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

    { "Narration", "And now she reads",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 16 },

    { "Narration", "And then she says them out loud.",
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
      EVENT_STOP_SOUNDS, BG_NONE, AVATAR_NONE, SOUND_TALKING,
      INSPECT_NONE, 0, {}, 21 },

    { "Narration", "Her voice is quieter than you remember.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 22 },

    { "Her", "You know how much I hate this... right?",
      EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_ELEVATOR_SCARY,
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
      INSPECT_NONE, 0, {}, 58 },

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
      INSPECT_NONE, 0, {}, 103 },

    { "Narration", "The memory fades.",
      EVENT_EYES_CLOSE | EVENT_DIALOG_HIDE | EVENT_AVATAR_HIDE | EVENT_GO_LEVEL3,
      BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, -1 }
};
const int LEVEL2_TEMPLATE_COUNT = ARRAY_COUNT(LEVEL2_TEMPLATE);

const DialogNode LEVEL3_TEMPLATE[] =
{
{ "Narration", "The elevator jerks to a stop before you can steady your breathing.",
      EVENT_CHANGE_BACKGROUND | EVENT_PLAY_SOUND, BG_INSIDE, AVATAR_NONE, SOUND_ELEVATOR_SCARY, INSPECT_NONE, 0, {}, 1 },

    { "Narration", "Your phone vibrates in your pocket.",
      EVENT_PLAY_SOUND, BG_NONE, AVATAR_NONE, SOUND_CALL, INSPECT_NONE, 0, {}, 2 },

    { "Friend", "Hey. Please pick up.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 3 },

    { "Friend", "You've been shutting everyone out since the break up.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 4 },

    { "Friend", "I know you're hurting, but disappearing like this isn't helping you.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 5 },

    { "You", "I don't know what to say anymore.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 6 },

    { "Friend", "Then start with something honest.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 7 },

    { "Friend", "Fight for yourself for once.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 8 },

    { "Narration", "The words hit harder than they should.",
      EVENT_SHAKE_SCREEN, BG_NONE, AVATAR_NONE, SOUND_RUMBLE, INSPECT_NONE, 0, {}, 9 },

    { "Narration", "In your head, the conversation twists into something uglier.",
      EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_L2_DINER_BOOTH, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 10 },

    { "Narration", "Not your friend.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 11 },

    { "Narration", "Just the part of you that would rather lash out than listen.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE, 0, {}, 12 },

    { "You", "Then fine. I'll fight it.",
      EVENT_AVATAR_SHOW | EVENT_FADE_OUT, BG_NONE, AVATAR_NEUTRAL, SOUND_NONE, INSPECT_NONE, 0, {}, -1 }
};
const int LEVEL3_TEMPLATE_COUNT = ARRAY_COUNT(LEVEL3_TEMPLATE);

const DialogNode LEVEL4_TEMPLATE[] =
{
    { "Narration", "The next floor greets you with a quiet that feels almost kind.",
      EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_INSIDE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 1 },

    { "You", "No shouting. No panic. Just... emptiness.",
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
const int LEVEL4_TEMPLATE_COUNT = ARRAY_COUNT(LEVEL4_TEMPLATE);

const DialogNode GOOD_ENDING_TEMPLATE[] =
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
const int GOOD_ENDING_TEMPLATE_COUNT = ARRAY_COUNT(GOOD_ENDING_TEMPLATE);

const DialogNode NEUTRAL_ENDING_TEMPLATE[] =
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
const int NEUTRAL_ENDING_TEMPLATE_COUNT = ARRAY_COUNT(NEUTRAL_ENDING_TEMPLATE);

const DialogNode BAD_ENDING_TEMPLATE[] =
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
const int BAD_ENDING_TEMPLATE_COUNT = ARRAY_COUNT(BAD_ENDING_TEMPLATE);