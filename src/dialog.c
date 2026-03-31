/* Reusable dialogue system.
   It handles the typewriter effect, branching choices, and drawing the text box. */

#include "dialog.h"
#include <string.h>

/* Helper used by the dialogue box.
   Splits long text into multiple lines so it stays inside the dialogue panel. */
static void DrawWrappedText(const char *text, float x, float y, float maxWidth, float fontSize, float spacing, Color color)
{
    Font font = GetFontDefault();

    int len = strlen(text);
    int start = 0;

    char line[512];

    Vector2 pos = {x, y};

    while(start < len)
    {
        int end = start;
        int lastSpace = -1;

        while(end < len)
        {
            if(text[end] == ' ')
                lastSpace = end;

            int count = end - start + 1;
            if(count >= 511) break;

            strncpy(line, &text[start], count);
            line[count] = '\0';

            Vector2 size = MeasureTextEx(font, line, fontSize, spacing);

            if(size.x > maxWidth)
            {
                if(lastSpace != -1 && lastSpace > start)
                    end = lastSpace;
                break;
            }

            end++;
        }

        int count = end - start;
        if(count <= 0) count = 1;

        strncpy(line, &text[start], count);
        line[count] = '\0';

        DrawTextEx(font, line, pos, fontSize, spacing, color);

        pos.y += fontSize + 5;

        start = end;

        // skip space after wrap
        if(text[start] == ' ') start++;
    }
}

/* Start reading a new dialogue script from node 0. */
void DialogStart(DialogState *state, DialogNode *script)
{
    state->nodes = script;
    state->index = 0;

    state->finished = false;
    state->waitingEvent = false;

    state->visibleChars = 0;
    state->typeTimer = 0;

    state->selectedChoice = 0;
}

/* Advance the active dialogue.
   This handles typing, input, choices, and returns scene events when a node triggers one. */
DialogEvent DialogUpdate(DialogState *state)
{
    if(state->finished) return EVENT_NONE;
    if(state->waitingEvent) return EVENT_NONE;

    DialogNode *node = &state->nodes[state->index];

    int len = strlen(node->text);

    /* typewriter */
    state->typeTimer += GetFrameTime() * 40;

    if(state->visibleChars < len)
        state->visibleChars = (int)state->typeTimer;

    if(state->visibleChars > len)
        state->visibleChars = len;

    /* no choices */
    if(node->choiceCount == 0)
    {
        if(IsKeyPressed(KEY_ENTER))
        {
            if(state->visibleChars < len)
            {
                state->visibleChars = len;
            }
            else
            {
                if(node->event != EVENT_NONE)
                {
                    // Pause dialogue progression until the event system says the current scene effect is done.
                    state->waitingEvent = true;

                    DialogEvent ev = node->event;
                    // Clear the event after firing it once so the same node does not replay it repeatedly.
                    node->event = EVENT_NONE;

                    return ev;
                }

                if(node->next == -1)
                {
                    state->finished = true;
                }
                else
                {
                    state->index = node->next;

                    state->visibleChars = 0;
                    state->typeTimer = 0;
                }
            }
        }
    }

    /* choices */
    else
    {
        if(IsKeyPressed(KEY_DOWN)) state->selectedChoice++;
        if(IsKeyPressed(KEY_UP)) state->selectedChoice--;

        if(state->selectedChoice < 0)
            state->selectedChoice = node->choiceCount - 1;

        if(state->selectedChoice >= node->choiceCount)
            state->selectedChoice = 0;

        if(IsKeyPressed(KEY_ENTER))
        {
            state->index = node->choices[state->selectedChoice].next;

            state->visibleChars = 0;
            state->typeTimer = 0;
            state->selectedChoice = 0;
        }
    }

    return EVENT_NONE;
}

/* Continue after an external event finishes.
   For example: fade, shake, inspect menu, or scene transition. */
void DialogResume(DialogState *state)
{
    DialogNode *node = &state->nodes[state->index];

    state->waitingEvent = false;

    if(node->next == -1)
        state->finished = true;
    else
        state->index = node->next;

    state->visibleChars = 0;
    state->typeTimer = 0;
}

/* Draw the current dialogue box, visible text, and choice list. */
void DialogDraw(DialogState *state)
{
    if(state->finished) return;
    if(state->waitingEvent) return;

    DialogNode *node = &state->nodes[state->index];

    int w = GetScreenWidth();
    int h = GetScreenHeight();

    DrawRectangle(200, h-400, w - 400, 400, Fade(BLACK, 0.7));

    DrawText(node->speaker, 240, h-380, 40, WHITE);

    char buffer[512];

    int max = sizeof(buffer) - 1;
    int count = state->visibleChars;
    if(count > max) count = max;

    strncpy(buffer, node->text, count);
    buffer[count] = '\0';

    // ✅ wrapped text
    DrawWrappedText(buffer, 240, h - 320, w - 480, 45, 2, WHITE);

    if(node->choiceCount > 0)
    {
        for(int i = 0; i < node->choiceCount; i++)
        {
            Color c = (i == state->selectedChoice) ? YELLOW : WHITE;

            DrawText(node->choices[i].text,
                     240,
                     h-220 + i * 50,
                     40,
                     c);
        }
    }
}