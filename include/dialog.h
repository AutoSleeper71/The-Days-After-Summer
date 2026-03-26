#ifndef DIALOG_H
#define DIALOG_H

#include "raylib.h"
#include "events.h"
#include <stdbool.h>

#define MAX_CHOICES 4

typedef struct
{
    const char *text;
    int next;
} DialogChoice;

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