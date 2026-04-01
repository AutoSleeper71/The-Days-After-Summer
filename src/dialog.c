/* Reusable dialogue system.
   It handles the typewriter effect, branching choices, and drawing the text box. */

#include "dialog.h"
#include <string.h>

#define DIALOG_FONT_SIZE        38.0f
#define DIALOG_LINE_SPACING      2.0f
#define NAME_FONT_SIZE          28.0f
#define CHOICE_FONT_SIZE        28.0f
#define FOOTER_FONT_SIZE        20.0f

/* Try to use a darker-looking UI font from assets if one exists.
   If no font file is present, safely fall back to raylib's default font. */
static Font GetDialogueFont(void)
{
    static bool initialized = false;
    static bool loaded = false;
    static Font font = {0};

    if (!initialized)
    {
        initialized = true;

        const char *candidates[] = {
            "assets/Cinzel-Regular.ttf",
            "assets/Cinzel.ttf",
            "assets/PlayfairDisplay-Regular.ttf",
            "assets/PlayfairDisplay.ttf",
            "assets/CrimsonText-Regular.ttf",
            "assets/CrimsonText.ttf",
            "assets/LibreBaskerville-Regular.ttf",
            "assets/LibreBaskerville.ttf",
            "assets/NotoSerif-Regular.ttf",
            "assets/NotoSerif.ttf",
            "assets/dialog_font.ttf",
            "assets/ui_font.ttf"
        };

        int count = (int)(sizeof(candidates) / sizeof(candidates[0]));
        for (int i = 0; i < count; i++)
        {
            if (FileExists(candidates[i]))
            {
                font = LoadFontEx(candidates[i], 64, 0, 0);
                loaded = (font.texture.id != 0);
                break;
            }
        }
    }

    if (loaded) return font;
    return GetFontDefault();
}

/* Helper used by the dialogue box.
   Splits long text into multiple lines so it stays inside the dialogue panel. */
static void DrawWrappedTextFont(Font font, const char *text, float x, float y, float maxWidth, float fontSize, float spacing, Color color)
{
    int len = (int)strlen(text);
    int start = 0;

    char line[512];
    Vector2 pos = { x, y };

    while (start < len)
    {
        int end = start;
        int lastSpace = -1;

        while (end < len)
        {
            if (text[end] == ' ') lastSpace = end;

            int count = end - start + 1;
            if (count >= 511) break;

            strncpy(line, &text[start], count);
            line[count] = '\0';

            Vector2 size = MeasureTextEx(font, line, fontSize, spacing);
            if (size.x > maxWidth)
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
        pos.y += fontSize + 7.0f;

        start = end;
        if (text[start] == ' ') start++;
    }
}

static void DrawTextShadow(Font font, const char *text, Vector2 pos, float size, float spacing, Color color)
{
    DrawTextEx(font, text, (Vector2){ pos.x + 2.0f, pos.y + 2.0f }, size, spacing, Fade(BLACK, 0.85f));
    DrawTextEx(font, text, pos, size, spacing, color);
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

    int len = (int)strlen(node->text);

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
                    state->waitingEvent = true;

                    DialogEvent ev = node->event;
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
    Font font = GetDialogueFont();

    int w = GetScreenWidth();
    int h = GetScreenHeight();

    float marginX = 150.0f;
    float boxH = 300.0f;
    float boxY = h - boxH - 70.0f;

    Rectangle outer = { marginX, boxY, w - marginX * 2.0f, boxH };
    Rectangle inner = { outer.x + 4.0f, outer.y + 4.0f, outer.width - 8.0f, outer.height - 8.0f };
    Rectangle namePlate = { outer.x + 18.0f, outer.y - 36.0f, 250.0f, 42.0f };

    DrawRectangleRounded(outer, 0.025f, 8, Fade(BLACK, 0.95f));
    DrawRectangleRounded(inner, 0.025f, 8, Fade((Color){18, 18, 24, 255}, 0.96f));
    DrawRectangleRoundedLinesEx(outer, 0.025f, 8, 2.5f, Fade(RAYWHITE, 0.85f));
    DrawRectangleRoundedLinesEx(inner, 0.025f, 8, 1.2f, Fade((Color){120, 120, 140, 255}, 0.40f));

    DrawRectangleRounded(namePlate, 0.14f, 8, Fade(BLACK, 0.96f));
    DrawRectangleRoundedLinesEx(namePlate, 0.14f, 8, 2.0f, Fade(RAYWHITE, 0.85f));


    Vector2 speakerPos = { namePlate.x + 16.0f, namePlate.y + 7.0f };
    DrawTextShadow(font, node->speaker, speakerPos, NAME_FONT_SIZE, 1.0f, RAYWHITE);

    char buffer[512];
    int max = (int)sizeof(buffer) - 1;
    int count = state->visibleChars;
    if(count > max) count = max;

    strncpy(buffer, node->text, count);
    buffer[count] = '\0';

    DrawWrappedTextFont(font,
                        buffer,
                        outer.x + 30.0f,
                        outer.y + 34.0f,
                        outer.width - 70.0f,
                        DIALOG_FONT_SIZE,
                        DIALOG_LINE_SPACING,
                        RAYWHITE);

    if(node->choiceCount > 0)
    {
        float choiceY = outer.y + outer.height - 86.0f - (node->choiceCount - 1) * 30.0f;

        for(int i = 0; i < node->choiceCount; i++)
        {
            bool selected = (i == state->selectedChoice);
            Color choiceColor = selected ? (Color){255, 245, 210, 255} : Fade(RAYWHITE, 0.88f);
            Color accent = selected ? (Color){220, 220, 220, 255} : Fade((Color){120, 120, 140, 255}, 0.65f);

            Rectangle choiceBox = {
                outer.x + 38.0f,
                choiceY + i * 38.0f,
                outer.width - 76.0f,
                30.0f
            };

            if (selected)
                DrawRectangleRounded(choiceBox, 0.18f, 6, Fade(WHITE, 0.08f));

            DrawRectangleLinesEx(choiceBox, 1.2f, accent);

            const char *prefix = selected ? ">" : " ";
            DrawTextEx(font,
                       TextFormat("%s %s", prefix, node->choices[i].text),
                       (Vector2){ choiceBox.x + 10.0f, choiceBox.y + 3.0f },
                       CHOICE_FONT_SIZE,
                       1.0f,
                       choiceColor);
        }
    }
    else if (state->visibleChars >= (int)strlen(node->text))
    {
        const char *continueText = "ENTER";
        Vector2 size = MeasureTextEx(font, continueText, FOOTER_FONT_SIZE, 1.0f);
        DrawTextEx(font,
                   continueText,
                   (Vector2){ outer.x + outer.width - size.x - 22.0f, outer.y + outer.height - 30.0f },
                   FOOTER_FONT_SIZE,
                   1.0f,
                   Fade(RAYWHITE, 0.78f));
    }
}
