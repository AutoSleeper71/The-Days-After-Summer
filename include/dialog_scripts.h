#ifndef DIALOG_SCRIPTS_H
#define DIALOG_SCRIPTS_H

#include "dialog.h"

/* Small helper for script + count pairs */
typedef struct
{
    const DialogNode *nodes;
    int count;
} DialogScript;

/* Elevator */
extern const DialogNode ELEVATOR_INTRO_TEMPLATE[];
extern const int ELEVATOR_INTRO_TEMPLATE_COUNT;

/* Main levels */
extern const DialogNode LEVEL1_TEMPLATE[];
extern const int LEVEL1_TEMPLATE_COUNT;

extern const DialogNode LEVEL2_TEMPLATE[];
extern const int LEVEL2_TEMPLATE_COUNT;

extern const DialogNode LEVEL3_TEMPLATE[];
extern const int LEVEL3_TEMPLATE_COUNT;

extern const DialogNode LEVEL4_TEMPLATE[];
extern const int LEVEL4_TEMPLATE_COUNT;

/* Endings */
extern const DialogNode GOOD_ENDING_TEMPLATE[];
extern const int GOOD_ENDING_TEMPLATE_COUNT;

extern const DialogNode NEUTRAL_ENDING_TEMPLATE[];
extern const int NEUTRAL_ENDING_TEMPLATE_COUNT;

extern const DialogNode BAD_ENDING_TEMPLATE[];
extern const int BAD_ENDING_TEMPLATE_COUNT;

/* Optional helper accessors */
DialogScript GetElevatorIntroScript(void);

DialogScript GetLevel1Script(void);
DialogScript GetLevel2Script(void);
DialogScript GetLevel3Script(void);
DialogScript GetLevel4Script(void);

DialogScript GetGoodEndingScript(void);
DialogScript GetNeutralEndingScript(void);
DialogScript GetBadEndingScript(void);

#endif