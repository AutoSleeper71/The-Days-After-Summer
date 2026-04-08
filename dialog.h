/* Public interface for the dialogue system. */

#ifndef DIALOG_H
#define DIALOG_H

#include "raylib.h"
#include "events.h"
#include <stdbool.h>

// Hard cap for how many options one dialogue node can present.
// File-specific compile-time limit used to size arrays safely.
#define MAX_CHOICES 4

/* One player choice inside a branching dialogue node. */
typedef struct
{
    const char *text;
    int next;
} DialogChoice;

/* One dialogue node.
   It stores text, optional scene events, optional choices, and the next node index. */
typedef struct
{
    const char *speaker;
    const char *text;

    DialogEvent event;

    int backgroundId;
    int avatarId;
    int soundId;
    int inspectId;

    int choiceCount;
    DialogChoice choices[MAX_CHOICES];

    int next;
} DialogNode;

/* Runtime state while a script is currently being played. */
typedef struct
{
    DialogNode *nodes;

    int index;

    bool finished;
    bool waitingEvent;

    float typeTimer;
    int visibleChars;

    int selectedChoice;
} DialogState;

void DialogStart(DialogState *state, DialogNode *script);
DialogEvent DialogUpdate(DialogState *state);
void DialogResume(DialogState *state);
void DialogDraw(DialogState *state);

#endif