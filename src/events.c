/* Shared scene event system.
   Centralizes backgrounds, avatars, overlays, sounds, transitions, fade effects, and inspect menus. */

#include "events.h"
#include <stddef.h>
#include <string.h>

// These pointers always describe what the current scene should draw right now.
static Texture2D *currentBG = NULL;
static Texture2D *currentAvatar = NULL;
static Texture2D *currentInspectTexture = NULL;

static bool resourcesLoaded = false;
static bool dialogVisible = true;
static bool blockInput = false;

// Internal transition modes used by the event system when moving between scenes.
typedef enum { TRANS_NONE, TRANS_SLIDE, TRANS_FLASH } TransitionMode;
static TransitionMode transitionMode = TRANS_NONE;
static float transitionTimer = 0.0f;

static bool shaking = false;
static float shakeTimer = 0.0f;
static Vector2 shakeOffset = {0};

static int fadeMode = 0;
static float fadeAlpha = 0.0f;

static int eyeMode = 0;
static float eyeTimer = 0.0f;
static float eyeClosedAmount = 0.0f;

static bool transitionPending = false;
static GameState pendingState = MENU;

// all the resources used by events, loaded at the start of the game and freed at the end

static Texture2D bgOutside, bgInside, bgRoom, bgBathroom, bgLobby;
static Texture2D bgNeutralEnding, bgHappyEnding, bgBadEnding;
static Texture2D bgL1Diner, bgL1Booth, bgL1Her, bgL1DinerEmpty, bgL1Park, bgL1ParkEmpty, bgL1Hospital;
static Texture2D bgL2Diner, bgL2Booth;
static Texture2D avatarNeutral, avatarConfused, avatarSad, avatarGirlHappy, avatarGirlDisappointed;
static Texture2D imgMirror, imgToothbrush, imgDuck;
static Sound sndMirror, sndToothbrush, sndDuck;
static Sound sndDing, sndRumble, sndScary, sndElevator, sndLight, sndTalking, sndPark, sndCall;
static Sound sndGoodEnd, sndNeutEnd;
static Texture2D imgSpaghetti, imgSteak, imgBurger, imgLetter;
static Texture2D imgSandwich, imgLemonade;

// inspecting system

/* Generic data bundle for inspect-style overlays.
   One struct can describe different inspect menus without duplicating code. */
typedef struct
{
    const char **items;
    const char **descriptions;
    Texture2D **images;
    Sound **sounds;
    int count;
    bool *checked;
} InspectMenu;

static bool inspecting = false;
static InspectMenu menu = {0};
static int menuIndex = 0;
static bool showingDesc = false;
static Texture2D *savedAvatar = NULL;

// bathroom inspection

static const char *bathroomItems[] = { "Mirror", "Toothbrush", "Rubber Duck" };
static const char *bathroomDesc[] = {
    "Your reflection looks tired.",
    "An old toothbrush. Replace it.",
    "A worn-out rubber duck."
};

static Texture2D *bathroomImages[] = { &imgMirror, &imgToothbrush, &imgDuck };
static Sound *bathroomSounds[] = { &sndMirror, &sndToothbrush, &sndDuck };
static bool bathroomChecked[] = { false, false, false };

// DINER INSPECTION

static const char *dinerItems[] = {
    "Spaghetti Bolognese",
    "Ribeye Steak",
    "Beef Burger",
    "Crumpled Letter"
};

static const char *dinerDesc[] = {
    "The spaghetti here is really good, always cooked al-dente, but its portion sizes are tiny.",
    "Steak here is pretty decent, but that price is just a little too much for 'decent'",
    "A good burger with a very large size. The problem is I am wearing a white suit. Not the best option for someone liable to such a mess.",
    "This shouldn't be here."
};

static Texture2D *dinerImages[] = {
    &imgSpaghetti, &imgSteak, &imgBurger, &imgLetter
};

static bool dinerChecked[4] = { false };

// BASKET INSPECTION

static const char *basketItems[] = {
    "Cheese Sandwich",
    "Bottle of Lemonade",
    "Crumpled Letter"
};

static const char *basketDesc[] = {
    "A perfect blend of seasoning and ingredients which genuinely surprises you. ",
    "All the muscles in your face and neck contract at the same time, in response to the extreme 'flavor'.",
    "Same strange letter..."
};

static Texture2D *basketImages[] = {
    &imgSandwich, &imgLemonade, &imgLetter
};

static bool basketChecked[3] = { false };

// custom overlay (card && letter)

typedef enum
{
    CUSTOM_NONE = 0,
    CUSTOM_CARD,
    CUSTOM_L2_LETTER
} CustomOverlayMode;

static CustomOverlayMode customMode = CUSTOM_NONE;
static const char *cardText = NULL;
static int customSelected = 0;
static bool customMessageActive = false;
static const char *customMessageTitle = NULL;
static const char *customMessageBody = NULL;
static bool l2LetterChecked = false;

/* Lightweight text wrapper used by inspect overlays and custom popups. */
static void DrawWrappedTextSimple(const char *text, float x, float y, float maxWidth, float fontSize, float spacing, Color color)
{
    Font font = GetFontDefault();
    int len = (int)strlen(text);
    int start = 0;
    char line[1024];
    Vector2 pos = { x, y };

    while (start < len)
    {
        int end = start;
        int lastSpace = -1;

        while (end < len)
        {
            if (text[end] == ' ') lastSpace = end;
            int count = end - start + 1;
            if (count >= 1023) break;

            strncpy(line, &text[start], count);
            line[count] = '\0';

            if (MeasureTextEx(font, line, fontSize, spacing).x > maxWidth)
            {
                if (lastSpace != -1 && lastSpace > start)
                    end = lastSpace;
                break;
            }
            end++;
        }

        int count = end - start;
        if (count <= 0) count = 1;

        strncpy(line, &text[start], count);
        line[count] = '\0';

        DrawTextEx(font, line, pos, fontSize, spacing, color);
        pos.y += fontSize + 6;

        start = end;
        if (text[start] == ' ') start++;
    }
}

/* Safe asset loader.
   Returns a zeroed texture if the file is missing so the game fails more gracefully. */
static bool FileLoadTexture(Texture2D *out, const char *path)
{
    if (!FileExists(path))
    {
        *out = (Texture2D){0};
        return false;
    }
    *out = LoadTexture(path);
    return true;
}

/* Safe sound loader with the same idea as FileLoadTexture. */
static bool FileLoadSound(Sound *out, const char *path)
{
    if (!FileExists(path))
    {
        *out = (Sound){0};
        return false;
    }
    *out = LoadSound(path);
    return true;
}

/* Map a background enum value from a dialogue node to the actual loaded texture. */
static void SetBackground(int id)
{
    switch(id)
    {
        case BG_OUTSIDE: currentBG = &bgOutside; break;
        case BG_INSIDE: currentBG = &bgInside; break;
        case BG_ROOM: currentBG = &bgRoom; break;
        case BG_BATHROOM: currentBG = &bgBathroom; break;
        case BG_LOBBY: currentBG = &bgLobby; break;
        case BG_NEUTRAL_ENDING: currentBG = &bgNeutralEnding; break;
        case BG_HAPPY_ENDING: currentBG = &bgHappyEnding; break;
        case BG_BAD_ENDING: currentBG = &bgBadEnding; break;
        case BG_L1_DINER: currentBG = &bgL1Diner; break;
        case BG_L1_DINER_BOOTH: currentBG = &bgL1Booth; break;
        case BG_L1_HER_CLOSEUP: currentBG = &bgL1Her; break;
        case BG_L1_DINER_EMPTY: currentBG = &bgL1DinerEmpty; break;
        case BG_L1_PARK: currentBG = &bgL1Park; break;
        case BG_L1_PARK_EMPTY: currentBG = &bgL1ParkEmpty; break;
        case BG_L1_HOSPITAL: currentBG = &bgL1Hospital; break;
        case BG_L2_DINER: currentBG = &bgL2Diner; break;
        case BG_L2_DINER_BOOTH: currentBG = &bgL2Booth; break;
        case BG_BLACK:
        case BG_NONE:
        default: currentBG = NULL; break;
    }
}

/* Map an avatar enum value from a dialogue node to the actual loaded portrait. */
static void SetAvatar(int id)
{
    switch(id)
    {
        case AVATAR_NEUTRAL: currentAvatar = &avatarNeutral; break;
        case AVATAR_CONFUSED: currentAvatar = &avatarConfused; break;
        case AVATAR_SAD: currentAvatar = &avatarSad; break;
        case AVATAR_GIRL_HAPPY: currentAvatar = &avatarGirlHappy; break;
        case AVATAR_GIRL_DISAPPOINTED: currentAvatar = &avatarGirlDisappointed; break;
        default: currentAvatar = NULL; break;
    }
}

/* Play one of the shared sounds requested by a dialogue/event node. */
static void PlaySoundById(int id)
{
    switch(id)
    {
        case SOUND_ELEVATOR_DING: if (sndDing.frameCount) PlaySound(sndDing); break;
        case SOUND_RUMBLE: if (sndRumble.frameCount) PlaySound(sndRumble); break;
        case SOUND_ELEVATOR_SCARY: if (sndScary.frameCount) PlaySound(sndScary); break;
        case SOUND_ELEVATOR: if (sndElevator.frameCount) PlaySound(sndElevator); break;
        case SOUND_LIGHT: if (sndLight.frameCount) PlaySound(sndLight); break;
        case SOUND_TALKING: if (sndTalking.frameCount) PlaySound(sndTalking); break;
        case SOUND_PARK: if (sndPark.frameCount) PlaySound(sndPark); break;
        case SOUND_CALL: if (sndCall.frameCount) PlaySound(sndCall); break;
        case SOUND_GOOD_END: if (sndGoodEnd.frameCount) PlaySound(sndGoodEnd); break;
        case SOUND_NEUT_END: if (sndNeutEnd.frameCount) PlaySound(sndNeutEnd); break;
        default: break;
    }
}

static void StopAllSharedSounds(void)
{
    if (sndDing.frameCount) StopSound(sndDing);
    if (sndRumble.frameCount) StopSound(sndRumble);
    if (sndScary.frameCount) StopSound(sndScary);
    if (sndElevator.frameCount) StopSound(sndElevator);
    if (sndLight.frameCount) StopSound(sndLight);
    if (sndTalking.frameCount) StopSound(sndTalking);
    if (sndPark.frameCount) StopSound(sndPark);
    if (sndCall.frameCount) StopSound(sndCall);
    if (sndGoodEnd.frameCount) StopSound(sndGoodEnd);
    if (sndNeutEnd.frameCount) StopSound(sndNeutEnd);
}

static bool AllChecked(const bool *values, int count)
{
    for (int i = 0; i < count; i++)
        if (!values[i]) return false;
    return true;
}

static void FinishOverlayAndResume(void)
{
    customMode = CUSTOM_NONE;
    cardText = NULL;
    customSelected = 0;
    customMessageActive = false;
    customMessageTitle = NULL;
    customMessageBody = NULL;
    dialogVisible = true;
    currentAvatar = savedAvatar;
    savedAvatar = NULL;
    blockInput = true;
}

static void OpenCustomMessage(const char *title, const char *body)
{
    customMessageActive = true;
    customMessageTitle = title;
    customMessageBody = body;
}

static void SetupInspect(int id)
{
    menuIndex = 0;
    showingDesc = false;
    currentInspectTexture = NULL;

    if(id == INSPECT_BATHROOM_ITEMS)
    {
        menu.items = bathroomItems;
        menu.descriptions = bathroomDesc;
        menu.images = bathroomImages;
        menu.count = 3;
        menu.checked = bathroomChecked;
        menu.sounds = bathroomSounds;
    }
    else if(id == INSPECT_L1_DINER_MENU)
    {
        menu.items = dinerItems;
        menu.descriptions = dinerDesc;
        menu.images = dinerImages;
        menu.count = 4;
        menu.checked = dinerChecked;
        menu.sounds = NULL; // optional
    }
    else if(id == INSPECT_L1_PARK_BASKET)
    {
        menu.items = basketItems;
        menu.descriptions = basketDesc;
        menu.images = basketImages;
        menu.count = 3;
        menu.checked = basketChecked;
        menu.sounds = NULL;
    }
}

static void StartCardById(int id)
{
    switch (id)
    {
        case CARD_L1_DAY: cardText = "The day it all started."; break;
        case CARD_L1_HORROR: cardText = "They taste better when they're scared you know?"; break;
        case CARD_L1_MELT: cardText = "Melt away...."; break;
        case CARD_L1_SLOW: cardText = "Take it slow."; break;
        case CARD_L1_LETTER: cardText = "It was a letter you were never supposed to write."; break;
        case CARD_L2_PROPOSAL: cardText = "\".....Will you marry me?\""; break;
        case CARD_L2_MEMORY:
            cardText = "\"You don't listen.\"\n\n\"You're pushing too hard.\"\n\n\"Why does everything have to be so serious?\"\n\n\"I've told you again and again, not right now.\"\n\n\"Please, let's just slow down.\"";
            break;
        default: cardText = ""; break;
    }

    customMode = CUSTOM_CARD;
    dialogVisible = false;
    savedAvatar = currentAvatar;
    currentAvatar = NULL;
}

static void HandleL2LetterSelect(void)
{
    l2LetterChecked = true;
    currentInspectTexture = &imgLetter;
    OpenCustomMessage(
        "Crumpled Letter",
        "The same letter that appeared from the place where you first met her, and your favorite place with her.\n\nThat same letter that was never supposed to be in those memories.\n\nInside reads a message that was too special to send by text, but a message that you could never say out loud.\n\nA message that made your throat dry and your heart race from the thought of saying the words.\n\nAnd so you wrote them down."
    ); 
}

static void StartCustomInspect(int id)
{
    customSelected = 0;
    customMessageActive = false;
    customMessageTitle = NULL;
    customMessageBody = NULL;

    switch (id)
    {
        case INSPECT_L2_LETTER:
            customMode = CUSTOM_L2_LETTER;

            dialogVisible = false;            
            savedAvatar = currentAvatar;     
            currentAvatar = NULL;            

            HandleL2LetterSelect();  
            break;

        default:
            customMode = CUSTOM_NONE;
            dialogVisible = true;
            currentAvatar = savedAvatar;
            savedAvatar = NULL;
            break;
    }
}

void EventsLoadResources(void)
{
    if(resourcesLoaded) return;
    resourcesLoaded = true;

    FileLoadTexture(&bgOutside, "assets/elevator_outside.jpg");
    FileLoadTexture(&bgInside, "assets/elevator_inside.jpg");
    FileLoadTexture(&bgRoom, "assets/room.jpg");
    FileLoadTexture(&bgBathroom, "assets/bathroom.png");
    FileLoadTexture(&bgLobby, "assets/charWithColleges.png");

    FileLoadTexture(&bgNeutralEnding, "assets/NeutralEnding.png");
    FileLoadTexture(&bgHappyEnding, "assets/HappyEnding.png");
    FileLoadTexture(&bgBadEnding, "assets/sad_ending.jpg");   

    FileLoadTexture(&bgL1Diner, "assets/Diner.png");
    FileLoadTexture(&bgL1Booth, "assets/Diner.png");
    FileLoadTexture(&bgL1Her, "assets/Diner.png");
    FileLoadTexture(&bgL1DinerEmpty, "assets/EmptyDiner.png");
    FileLoadTexture(&bgL1Park, "assets/MorningPark.png");
    FileLoadTexture(&bgL1ParkEmpty, "assets/EveningPark.png");
    FileLoadTexture(&bgL1Hospital, "assets/room.png");

    FileLoadTexture(&bgL2Diner, "assets/Diner.png");
    FileLoadTexture(&bgL2Booth, "assets/Diner.png");

    FileLoadTexture(&avatarNeutral, "assets/character.png");
    FileLoadTexture(&avatarConfused, "assets/character.png");
    FileLoadTexture(&avatarSad, "assets/HappyMan.png");
    FileLoadTexture(&avatarGirlHappy, "assets/Happy.png");
    FileLoadTexture(&avatarGirlDisappointed, "assets/Disapointed.png");

    FileLoadTexture(&imgMirror, "assets/mirror.png");
    FileLoadTexture(&imgToothbrush, "assets/Toothbrushes.png");
    FileLoadTexture(&imgDuck, "assets/rubberDuck.png");

    FileLoadTexture(&imgSpaghetti, "assets/Spaghetti.png");
    FileLoadTexture(&imgSteak, "assets/Steak.png");
    FileLoadTexture(&imgBurger, "assets/BeefBurger.png");
    FileLoadTexture(&imgLetter, "assets/letter.png");

    FileLoadTexture(&imgSandwich, "assets/Sandwich.png");
    FileLoadTexture(&imgLemonade, "assets/Lemonade.png");

    FileLoadSound(&sndDing, "assets/choose_option.mp3");
    FileLoadSound(&sndScary, "assets/elevatorScarySound.wav");
    FileLoadSound(&sndElevator, "assets/elevatorDing.wav");
    FileLoadSound(&sndLight, "assets/light.wav");
    FileLoadSound(&sndTalking, "assets/talking.wav");
    FileLoadSound(&sndCall, "assets/calling.mp3");
    FileLoadSound(&sndGoodEnd, "assets/goodending.mp3");
    FileLoadSound(&sndNeutEnd, "assets/neutralending.mp3");
    sndRumble = sndDing;
    FileLoadSound(&sndMirror, "assets/mirror.wav");
    FileLoadSound(&sndToothbrush, "assets/toothbrush.wav");
    FileLoadSound(&sndDuck, "assets/rubberDuck.wav");
    FileLoadSound(&sndPark, "assets/park.mp3");
}

void EventsInit(void)
{
    currentBG = &bgOutside;
    currentAvatar = NULL;
    currentInspectTexture = NULL;
    dialogVisible = true;
    blockInput = false;
    transitionMode = TRANS_NONE;
    transitionTimer = 0.0f;
    shaking = false;
    shakeTimer = 0.0f;
    shakeOffset = (Vector2){0};
    fadeMode = 0;
    fadeAlpha = 0.0f;
    eyeMode = 0;
    eyeTimer = 0.0f;
    eyeClosedAmount = 0.0f;
    transitionPending = false;
    pendingState = MENU;
    inspecting = false;
    menu = (InspectMenu){0};
    menuIndex = 0;
    showingDesc = false;
    customMode = CUSTOM_NONE;
    cardText = NULL;
    customSelected = 0;
    customMessageActive = false;
    customMessageTitle = NULL;
    customMessageBody = NULL;
    savedAvatar = NULL;

    memset(bathroomChecked, 0, sizeof(bathroomChecked));
    memset(dinerChecked, 0, sizeof(dinerChecked));
    memset(basketChecked, 0, sizeof(basketChecked));
    l2LetterChecked = false;
}

// all the events description

/* React to event flags attached to a dialogue node.
   A single node can request multiple things at once because events are stored as bit flags. */
void EventsTrigger(DialogEvent ev, int bg, int avatar, int sound, int inspectId)
{
    if(ev & EVENT_DIALOG_SHOW) dialogVisible = true;
    if(ev & EVENT_DIALOG_HIDE) dialogVisible = false;
    // Background/avatar/sound IDs are passed separately so the same event system can be reused by every level.
    if(ev & EVENT_CHANGE_BACKGROUND) SetBackground(bg);
    if(ev & EVENT_AVATAR_SHOW) SetAvatar(avatar);
    if(ev & EVENT_AVATAR_HIDE) currentAvatar = NULL;
    if(ev & EVENT_PLAY_SOUND) PlaySoundById(sound);
    if(ev & EVENT_STOP_SOUNDS) StopAllSharedSounds();

    if(ev & EVENT_FADE_IN) { fadeMode = 1; fadeAlpha = 1.0f; }
    if(ev & EVENT_FADE_OUT) { fadeMode = 2; fadeAlpha = 0.0f; }
    if(ev & EVENT_EYES_CLOSE) { eyeMode = 1; eyeTimer = 0.0f; }
    if(ev & EVENT_EYES_OPEN) { eyeMode = 2; eyeTimer = 0.0f; }
    if(ev & EVENT_SHAKE_SCREEN) { shaking = true; shakeTimer = 0.0f; }

    if(ev & EVENT_INSPECT_START)
    {
        // Special overlays temporarily take over input until the player closes them.
        if (inspectId == INSPECT_L2_LETTER)
        {
            StartCustomInspect(inspectId); 
        }
        else
        {
            SetupInspect(inspectId);        
            inspecting = true;
            dialogVisible = false;
            savedAvatar = currentAvatar;
            currentAvatar = NULL;
        }
    }

    if(ev & EVENT_INSPECT_END)
    {
        inspecting = false;
        FinishOverlayAndResume();
    }

    if(ev & EVENT_SHOW_CARD) StartCardById(inspectId);

    if(ev & EVENT_GO_LEVEL1 || ev & EVENT_GO_LEVEL2 ||
   ev & EVENT_GO_LEVEL3 || ev & EVENT_GO_LEVEL4 ||
   ev & EVENT_GO_ENDING)
    {
        transitionPending = true;

        if(ev & EVENT_GO_LEVEL1) pendingState = LEVEL1;
        if(ev & EVENT_GO_LEVEL2) pendingState = LEVEL2;
        if(ev & EVENT_GO_LEVEL3) pendingState = LEVEL3;
        if(ev & EVENT_GO_LEVEL4) pendingState = LEVEL4;
        if(ev & EVENT_GO_ENDING) pendingState = finalEnding;

        // deleted transitions cause now they are useless
        fadeMode = 0;
        fadeAlpha = 0.0f;

        transitionMode = TRANS_NONE;
    }

    if(ev & EVENT_TRANSITION_SLIDE) { transitionMode = TRANS_SLIDE; transitionTimer = 0.0f; }
    if(ev & EVENT_TRANSITION_FLASH) { transitionMode = TRANS_FLASH; transitionTimer = 0.0f; }
}

void EventsUpdate(void)
{
    float dt = GetFrameTime();

    if(transitionMode != TRANS_NONE)
    {
        transitionTimer += dt;
        if(transitionTimer >= 1.0f) transitionMode = TRANS_NONE;
    }

    if(shaking)
    {
        shakeTimer += dt;
        shakeOffset.x = (float)GetRandomValue(-5, 5);
        shakeOffset.y = (float)GetRandomValue(-5, 5);
        if(shakeTimer >= 2.0f) shaking = false;
    }
    else shakeOffset = (Vector2){0, 0};

    if(fadeMode == 1)
    {
        fadeAlpha -= dt;
        if(fadeAlpha <= 0.0f) { fadeAlpha = 0.0f; fadeMode = 0; }
    }
    else if(fadeMode == 2)
    {
        fadeAlpha += dt;
        if(fadeAlpha >= 1.0f) { fadeAlpha = 1.0f; fadeMode = 0; }
    }

    if(eyeMode == 1)
    {
        eyeTimer += dt;
        eyeClosedAmount = eyeTimer / 1.0f;
        if(eyeClosedAmount >= 1.0f) { eyeClosedAmount = 1.0f; eyeMode = 0; }
    }
    else if(eyeMode == 2)
    {
        eyeTimer += dt;
        eyeClosedAmount = 1.0f - (eyeTimer / 1.0f);
        if(eyeClosedAmount <= 0.0f) { eyeClosedAmount = 0.0f; eyeMode = 0; }
    }

    // inspecting input handling
    if(inspecting)
    {
        if(showingDesc)
        {
            if(IsKeyPressed(KEY_ENTER))
            {
                if(menu.sounds && menu.sounds[menuIndex] && menu.sounds[menuIndex]->frameCount)
                    StopSound(*menu.sounds[menuIndex]);

                showingDesc = false;

                if(AllChecked(menu.checked, menu.count))
                {
                    inspecting = false;
                    dialogVisible = true;
                    currentAvatar = savedAvatar;
                    savedAvatar = NULL;
                    blockInput = true;
                }
            }
        }
        else
        {
            if(IsKeyPressed(KEY_DOWN)) menuIndex = (menuIndex + 1) % menu.count;
            if(IsKeyPressed(KEY_UP)) menuIndex = (menuIndex - 1 + menu.count) % menu.count;

            if(IsKeyPressed(KEY_ENTER))
            {
                menu.checked[menuIndex] = true;
                showingDesc = true;
                currentInspectTexture = (menu.images ? menu.images[menuIndex] : NULL);

                if(menu.sounds && menu.sounds[menuIndex] && menu.sounds[menuIndex]->frameCount)
                    PlaySound(*menu.sounds[menuIndex]);
            }
        }
        return;
    }

    // custom inspecting (card && letter)

    if(customMode == CUSTOM_CARD)
    {
        if(IsKeyPressed(KEY_ENTER)) FinishOverlayAndResume();
        return;
    }

    if(customMode == CUSTOM_L2_LETTER)
    {
        if(customMessageActive)
        {
            if(IsKeyPressed(KEY_ENTER)) FinishOverlayAndResume();
        }
        else if(IsKeyPressed(KEY_ENTER))
        {
            HandleL2LetterSelect();
        }
        return;
    }
}

// drawing the inspection

void EventsDrawOverlay(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if(transitionMode == TRANS_SLIDE)
    {
        int offset = (int)(w * (1.0f - transitionTimer));
        DrawRectangle(-offset, 0, w, h, BLACK);
    }

    if(transitionMode == TRANS_FLASH)
        DrawRectangle(0, 0, w, h, Fade(WHITE, 1.0f - transitionTimer));

    if(fadeAlpha > 0.0f)
        DrawRectangle(0, 0, w, h, Fade(BLACK, fadeAlpha));

    if(eyeClosedAmount > 0.0f)
    {
        int lid = (int)((h / 2.0f) * eyeClosedAmount);
        DrawRectangle(0, 0, w, lid, BLACK);
        DrawRectangle(0, h - lid, w, lid, BLACK);
    }

    if(inspecting)
    {
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.85f));
        DrawText("Inspect", 50, 50, 30, WHITE);

        for(int i = 0; i < menu.count; i++)
        {
            Color c = (i == menuIndex) ? YELLOW : WHITE;
            DrawText(menu.items[i], 80, 120 + i * 40, 28, c);
            if(menu.checked[i]) DrawText("[x]", 300, 120 + i * 40, 28, GREEN);
        }

        if(showingDesc)
        {
            Texture2D *img = currentInspectTexture;

            if(img && img->id != 0)
            {
                float scale = 0.4f;
                DrawTextureEx(*img, (Vector2){ w/2 - img->width*scale/2, h/2 - 150 }, 0, scale, WHITE);
            }

            DrawRectangle(50, h - 200, w - 100, 150, Fade(BLACK, 0.95f));
            DrawWrappedTextSimple(menu.descriptions[menuIndex], 70, h - 160, w - 140, 22, 2, WHITE);
        }
        return;
    }

    if(customMode == CUSTOM_CARD)
    {
        DrawRectangle(0, 0, w, h, BLACK);
        DrawWrappedTextSimple(cardText ? cardText : "", 180, h/2 - 60, w - 360, 46, 2, WHITE);
        DrawText("Press ENTER", w/2 - 90, h - 100, 30, GRAY);
        return;
    }

    if(customMode == CUSTOM_L2_LETTER)
    {
        DrawRectangle(100, 120, w - 200, h - 240, Fade(BLACK, 0.88f));

        Texture2D *img = currentInspectTexture;

        if(img && img->id != 0)
        {
            float scale = 0.4f;
            DrawTextureEx(*img,
                (Vector2){ w/2 - img->width*scale/2, h/2 - 50 },
                0, scale, WHITE);
        }

        if(customMessageActive)
        {
            DrawRectangle(140, h - 360, w - 280, 250, Fade(BLACK, 0.95f));

            if(customMessageTitle) DrawText(customMessageTitle, 170, h - 330, 32, YELLOW);
            if(customMessageBody) DrawWrappedTextSimple(customMessageBody, 170, h - 285, w - 340, 23, 2, WHITE);

            DrawText("Press ENTER", w - 280, h - 145, 24, GRAY);
        }
    }
}

// blocking input when needed, checking if a transition can happen, and getters for the current state of events

bool EventsBusy(void)
{
    return inspecting || (customMode != CUSTOM_NONE) || (transitionMode != TRANS_NONE) || shaking || (fadeMode != 0) || (eyeMode != 0);
}

bool EventsShouldBlockInput(void)
{
    if(blockInput)
    {
        blockInput = false;
        return true;
    }
    return false;
}

/* Return a pending scene change exactly once, then clear it. */
bool EventsConsumeTransition(GameState *outState)
{
    if(transitionPending && !EventsBusy())
    {
        if(outState) *outState = pendingState;
        transitionPending = false;
        return true;
    }
    return false;
}

//idk if we still need all these getters but here they are ill delete it later

bool EventsIsDialogVisible(void) { return dialogVisible; }
Texture2D *EventsGetCurrentBackground(void) { return currentBG; }
Texture2D *EventsGetCurrentAvatar(void) { return currentAvatar; }
Vector2 EventsGetShakeOffset(void) { return shakeOffset; }
Texture2D *EventsGetCurrentInspect(void) { return currentInspectTexture; }
