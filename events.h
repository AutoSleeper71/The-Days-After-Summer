/* Public interface for the shared event / overlay system. */

#ifndef EVENTS_H
#define EVENTS_H

#include "raylib.h"
#include "game.h"
#include <stdbool.h>

// Extra transition effect flags.
#define EVENT_TRANSITION_SLIDE  (1 << 20)
#define EVENT_TRANSITION_FLASH  (1 << 21)

/* Bit-flag events that a dialogue node can request from the scene system. */
typedef enum
{
    EVENT_NONE              = 0,
    EVENT_FADE_IN           = 1 << 0,
    EVENT_FADE_OUT          = 1 << 1,
    EVENT_EYES_CLOSE        = 1 << 2,
    EVENT_EYES_OPEN         = 1 << 3,
    EVENT_DIALOG_SHOW       = 1 << 4,
    EVENT_DIALOG_HIDE       = 1 << 5,
    EVENT_AVATAR_SHOW       = 1 << 6,
    EVENT_AVATAR_HIDE       = 1 << 7,
    EVENT_PLAY_SOUND        = 1 << 8,
    EVENT_CHANGE_BACKGROUND = 1 << 9,
    EVENT_SHAKE_SCREEN      = 1 << 10,
    EVENT_GO_LEVEL1         = 1 << 11,
    EVENT_GO_LEVEL2         = 1 << 12,
    EVENT_GO_LEVEL3         = 1 << 13,
    EVENT_GO_LEVEL4         = 1 << 14,
    EVENT_GO_ENDING         = 1 << 15,
    EVENT_INSPECT_START     = 1 << 16,
    EVENT_INSPECT_END       = 1 << 17,
    EVENT_STOP_SOUNDS       = 1 << 18,
    EVENT_SHOW_CARD         = 1 << 19
} DialogEvent;

/* Shared background IDs understood by the event system. */
typedef enum
{
    BG_NONE = 0,
    BG_OUTSIDE,
    BG_INSIDE,
    BG_ROOM,
    BG_BATHROOM,
    BG_LOBBY,
    BG_NEUTRAL_ENDING,
    BG_HAPPY_ENDING,
    BG_BAD_ENDING,
    BG_LOBBY_BACK,
    BG_BOOTH,

    BG_L1_DINER,
    BG_L1_DINER_BOOTH,
    BG_L1_HER_CLOSEUP,
    BG_L1_DINER_EMPTY,
    BG_L1_PARK,
    BG_L1_PARK_EMPTY,
    BG_L1_HOSPITAL,

    BG_L2_DINER,
    BG_L2_DINER_BOOTH,

    BG_BLACK
} EventBackground;

/* Shared portrait IDs. */
typedef enum
{
    AVATAR_NONE = 0,
    AVATAR_NEUTRAL,
    AVATAR_CONFUSED,
    AVATAR_SAD,
    AVATAR_GIRL_HAPPY,
    AVATAR_GIRL_DISAPPOINTED
} EventAvatar;

/* Shared sound effect / music IDs. */
typedef enum
{
    SOUND_NONE = 0,
    SOUND_ELEVATOR_DING,
    SOUND_RUMBLE,
    SOUND_ELEVATOR_SCARY,
    SOUND_ELEVATOR,
    SOUND_LIGHT,
    SOUND_TALKING,
    SOUND_PARK,
    SOUND_CALL,
    SOUND_GOOD_END,
    SOUND_NEUT_END
} EventSound;

/* Overlay/inspect IDs used by special interactive dialogue nodes. */
typedef enum
{
    INSPECT_NONE = 0,

    INSPECT_BATHROOM_ITEMS = 20,

    INSPECT_L1_DINER_MENU,
    INSPECT_L1_PARK_BASKET,
    INSPECT_L2_LETTER,

    CARD_L1_DAY,
    CARD_L1_HORROR,
    CARD_L1_MELT,
    CARD_L1_SLOW,
    CARD_L1_LETTER,
    CARD_L2_PROPOSAL,
    CARD_L2_MEMORY
} InspectId;

void EventsLoadResources(void);
void EventsInit(void);
void EventsTrigger(DialogEvent ev, int backgroundId, int avatarId, int soundId, int inspectId);
void EventsUpdate(void);
void EventsDrawOverlay(void);

bool EventsBusy(void);
bool EventsShouldBlockInput(void);
bool EventsIsDialogVisible(void);
bool EventsConsumeTransition(GameState *outState);

Texture2D *EventsGetCurrentBackground(void);
Texture2D *EventsGetCurrentAvatar(void);
Vector2 EventsGetShakeOffset(void);
Texture2D *EventsGetCurrentInspect(void);

#endif
