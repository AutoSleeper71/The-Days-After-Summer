#include "level3.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"

#include <string.h>
#include <stdbool.h>

#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define MAX_LEVEL3_NODES 96

#define PLAYER_MAX_HP 100
#define ENEMY_MAX_HP 90

typedef enum
{
    LEVEL3_PHASE_DIALOG = 0,
    LEVEL3_PHASE_RPG,
    LEVEL3_PHASE_RESULT
} Level3Phase;

typedef struct
{
    int playerHp;
    int enemyHp;
    int playerGuard;
    int healCharges;
    bool playerTurn;
    bool finished;
    bool playerWon;
    int turnNumber;
    char message[256];
} Level3Battle;

static bool level3Initialized = false;
static DialogState level3Dialog;
static DialogNode activeNodes[MAX_LEVEL3_NODES];
static bool waitingOnEvent = false;
static Level3Phase level3Phase = LEVEL3_PHASE_DIALOG;
static Level3Battle battle;

static Texture2D bgBattle;
static Texture2D texPlayer;
static Texture2D texEnemy;
static bool artLoaded = false;

static bool TryLoadTexture(Texture2D *out, const char *path)
{
    if (!FileExists(path))
    {
        *out = (Texture2D){0};
        return false;
    }

    *out = LoadTexture(path);
    return true;
}

static void EnsureArtLoaded(void)
{
    if (artLoaded) return;

    TryLoadTexture(&bgBattle, "assets/EmptyDiner.png");
    if (bgBattle.id == 0) TryLoadTexture(&bgBattle, "assets/Diner.png");

    TryLoadTexture(&texPlayer, "assets/character.png");
    if (texPlayer.id == 0) TryLoadTexture(&texPlayer, "assets/HappyMan.png");

    TryLoadTexture(&texEnemy, "assets/Disapointed.png");
    if (texEnemy.id == 0) TryLoadTexture(&texEnemy, "assets/Happy.png");

    artLoaded = true;
}

static void CopyScript(const DialogNode *src, int count)
{
    if (count > MAX_LEVEL3_NODES) count = MAX_LEVEL3_NODES;
    memcpy(activeNodes, src, sizeof(DialogNode) * count);
}

static bool IsProtagonistSpeaker(const char *speaker)
{
    return (strcmp(speaker, "You") == 0) ||
           (strcmp(speaker, "Daniel") == 0) ||
           (strcmp(speaker, "Him") == 0);
}

static bool IsFriendSpeaker(const char *speaker)
{
    return strcmp(speaker, "Friend") == 0;
}

static int AvatarForSpeaker(const char *speaker)
{
    if (IsProtagonistSpeaker(speaker)) return AVATAR_NEUTRAL;
    if (IsFriendSpeaker(speaker)) return AVATAR_GIRL_HAPPY;
    return AVATAR_NONE;
}

static void ApplyAvatarPreload(DialogNode *nodes, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        DialogNode *cur = &nodes[i];
        DialogNode *next = &nodes[i + 1];
        int nextAvatar = AvatarForSpeaker(next->speaker);

        if (nextAvatar != AVATAR_NONE && (cur->event & EVENT_AVATAR_HIDE) == 0)
        {
            cur->event |= EVENT_AVATAR_SHOW;
            cur->avatarId = nextAvatar;
        }
    }
}

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

static const DialogNode level3Template[] =
{
    { "Narration", "The elevator jerks to a stop before you can steady your breathing.",
      EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_INSIDE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 1 },

    { "Narration", "Your phone vibrates in your pocket.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 2 },

    { "Friend", "Hey. Please pick up.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 3 },

    { "Friend", "You've been shutting everyone out since the break up.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 4 },

    { "Friend", "I know you're hurting, but disappearing like this isn't helping you.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 5 },

    { "You", "I don't know what to say anymore.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 6 },

    { "Friend", "Then start with something honest.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 7 },

    { "Friend", "Fight for yourself for once.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 8 },

    { "Narration", "The words hit harder than they should.",
      EVENT_SHAKE_SCREEN, BG_NONE, AVATAR_NONE, SOUND_RUMBLE,
      INSPECT_NONE, 0, {}, 9 },

    { "Narration", "In your head, the conversation twists into something uglier.",
      EVENT_CHANGE_BACKGROUND | EVENT_FADE_IN, BG_L2_DINER_BOOTH, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 10 },

    { "Narration", "Not your friend. Not really.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 11 },

    { "Narration", "Just the part of you that would rather lash out than listen.",
      EVENT_NONE, BG_NONE, AVATAR_NONE, SOUND_NONE,
      INSPECT_NONE, 0, {}, 12 },

    { "You", "Then fine. I'll fight it.",
      EVENT_AVATAR_SHOW | EVENT_FADE_OUT, BG_NONE, AVATAR_NEUTRAL, SOUND_NONE,
      INSPECT_NONE, 0, {}, -1 }
};

static void StartBattle(void)
{
    battle.playerHp = PLAYER_MAX_HP;
    battle.enemyHp = ENEMY_MAX_HP;
    battle.playerGuard = 0;
    battle.healCharges = 3;
    battle.playerTurn = true;
    battle.finished = false;
    battle.playerWon = false;
    battle.turnNumber = 1;
    strcpy(battle.message, "Your turn. A=Attack  G=Guard  H=Heal");
}

static void InitLevel3State(void)
{
    int count = ARRAY_COUNT(level3Template);

    waitingOnEvent = false;
    level3Phase = LEVEL3_PHASE_DIALOG;

    EnsureArtLoaded();
    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    CopyScript(level3Template, count);
    ApplyAvatarPreload(activeNodes, count);

    if (!ValidateScript(activeNodes, count))
    {
        TraceLog(LOG_ERROR, "Level3 dialog script has invalid indices.");
        level3Initialized = false;
        return;
    }

    DialogStart(&level3Dialog, activeNodes);
    StartBattle();
    level3Initialized = true;
}

static void DrawBattleTexture(Texture2D tex, Rectangle dest, Color tint)
{
    if (tex.id != 0)
    {
        DrawTexturePro(tex,
            (Rectangle){0, 0, (float)tex.width, (float)tex.height},
            dest,
            (Vector2){0, 0},
            0.0f,
            tint);
    }
    else
    {
        DrawRectangleRounded(dest, 0.15f, 8, Fade(tint, 0.9f));
    }
}

static void DrawHealthBar(int x, int y, int width, int hp, int maxHp, const char *label)
{
    DrawText(label, x, y - 28, 24, WHITE);
    DrawRectangle(x, y, width, 22, Fade(WHITE, 0.18f));

    if (hp < 0) hp = 0;
    int fill = (int)((float)width * ((float)hp / (float)maxHp));
    DrawRectangle(x, y, fill, 22, RED);
    DrawRectangleLines(x, y, width, 22, WHITE);

    DrawText(TextFormat("%d / %d", hp, maxHp), x + width + 20, y - 2, 20, WHITE);
}

static void EnemyTurn(void)
{
    int actionRoll = GetRandomValue(0, 99);
    int damage = 0;

    if (battle.enemyHp <= 25 && actionRoll < 35)
    {
        damage = 22;
        strcpy(battle.message, "The memory hits back hard.");
    }
    else if (actionRoll < 70)
    {
        damage = 14;
        strcpy(battle.message, "The memory lashes out.");
    }
    else
    {
        damage = 8;
        strcpy(battle.message, "The memory hesitates, but still hurts you.");
    }

    if (battle.playerGuard > 0)
    {
        damage /= 2;
        battle.playerGuard = 0;
        strcat(battle.message, " Your guard softens the blow.");
    }

    battle.playerHp -= damage;
    if (battle.playerHp <= 0)
    {
        battle.playerHp = 0;
        battle.finished = true;
        battle.playerWon = false;
        strcpy(battle.message, "You collapse under the weight of your anger.");
        level3Phase = LEVEL3_PHASE_RESULT;
        return;
    }

    battle.playerTurn = true;
    battle.turnNumber++;
}

static void UpdateBattle(void)
{
    if (battle.finished) return;

    if (battle.playerTurn)
    {
        if (IsKeyPressed(KEY_A))
        {
            int damage = GetRandomValue(16, 24);
            battle.enemyHp -= damage;
            strcpy(battle.message, TextFormat("You steady yourself and deal %d damage.", damage));
            battle.playerTurn = false;
        }
        else if (IsKeyPressed(KEY_G))
        {
            battle.playerGuard = 1;
            strcpy(battle.message, "You brace for the next hit.");
            battle.playerTurn = false;
        }
        else if (IsKeyPressed(KEY_H))
        {
            if (battle.healCharges > 0)
            {
                int heal = GetRandomValue(14, 22);
                battle.playerHp += heal;
                if (battle.playerHp > PLAYER_MAX_HP) battle.playerHp = PLAYER_MAX_HP;
                battle.healCharges--;
                strcpy(battle.message, TextFormat("You recover %d HP.", heal));
                battle.playerTurn = false;
            }
            else
            {
                strcpy(battle.message, "No heals left.");
            }
        }

        if (!battle.playerTurn)
        {
            if (battle.enemyHp <= 0)
            {
                battle.enemyHp = 0;
                battle.finished = true;
                battle.playerWon = true;
                strcpy(battle.message, "You break through the spiral and reclaim your voice.");
                level3Phase = LEVEL3_PHASE_RESULT;
            }
        }
    }
    else
    {
        EnemyTurn();
    }
}

static void DrawBattleScene(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if (bgBattle.id != 0)
        DrawTexturePro(bgBattle,
            (Rectangle){0, 0, (float)bgBattle.width, (float)bgBattle.height},
            (Rectangle){0, 0, (float)w, (float)h},
            (Vector2){0, 0},
            0.0f,
            WHITE);
    else
        DrawRectangle(0, 0, w, h, BLACK);

    DrawRectangle(0, 0, w, h, Fade(BLACK, 0.45f));
    DrawText("LEVEL 3 - ANGER", 80, 50, 42, WHITE);
    DrawText("Fight the version of yourself that wants to lash out.", 80, 100, 24, LIGHTGRAY);

    DrawHealthBar(90, 170, 320, battle.playerHp, PLAYER_MAX_HP, "Daniel");
    DrawHealthBar(w - 470, 170, 320, battle.enemyHp, ENEMY_MAX_HP, "Spiral");

    DrawBattleTexture(texPlayer,
        (Rectangle){120, 250, 360, 520},
        WHITE);
    DrawBattleTexture(texEnemy,
        (Rectangle){w - 500, 250, 320, 460},
        WHITE);

    DrawRectangle(70, h - 250, w - 140, 180, Fade(BLACK, 0.78f));
    DrawRectangleLines(70, h - 250, w - 140, 180, Fade(WHITE, 0.35f));

    DrawText(TextFormat("Turn %d", battle.turnNumber), 100, h - 220, 26, YELLOW);
    DrawText(TextFormat("Heals left: %d", battle.healCharges), 260, h - 220, 26, SKYBLUE);
    DrawText(battle.message, 100, h - 180, 28, WHITE);

    if (!battle.finished)
    {
        Color promptColor = battle.playerTurn ? WHITE : GRAY;
        DrawText("A - Attack", 100, h - 125, 28, promptColor);
        DrawText("G - Guard", 320, h - 125, 28, promptColor);
        DrawText("H - Heal", 520, h - 125, 28, promptColor);

        if (!battle.playerTurn)
            DrawText("Enemy turn...", w - 320, h - 125, 28, ORANGE);
    }
}

void InitLevel3(void)
{
    level3Initialized = false;
}

GameState UpdateLevel3(void)
{
    if (!level3Initialized)
        InitLevel3State();

    if (!level3Initialized)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
        DrawText("Level 3 failed to initialize", 40, 40, 24, RED);
        return LEVEL3;
    }

    if (level3Phase == LEVEL3_PHASE_DIALOG)
    {
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

        if (waitingOnEvent && !level3Dialog.finished && !EventsBusy())
        {
            waitingOnEvent = false;
            DialogResume(&level3Dialog);
        }

        if (!level3Dialog.finished)
        {
            if (EventsShouldBlockInput())
                return LEVEL3;

            if (!waitingOnEvent)
            {
                DialogEvent ev = DialogUpdate(&level3Dialog);

                if (ev != EVENT_NONE)
                {
                    DialogNode *node = &level3Dialog.nodes[level3Dialog.index];
                    EventsTrigger(ev, node->backgroundId, node->avatarId, node->soundId, node->inspectId);
                    waitingOnEvent = true;
                }
            }

            if (EventsIsDialogVisible())
                DialogDraw(&level3Dialog);
        }
        else
        {
            level3Phase = LEVEL3_PHASE_RPG;
        }

        EventsDrawOverlay();
        return LEVEL3;
    }

    if (level3Phase == LEVEL3_PHASE_RPG)
    {
        UpdateBattle();
        DrawBattleScene();
        return LEVEL3;
    }

    DrawBattleScene();

    if (battle.playerWon)
    {
        DrawText("Press ENTER to ride to the next floor", GetScreenWidth()/2 - 240, 40, 28, YELLOW);

        if (IsKeyPressed(KEY_ENTER))
        {
            if (battle.playerHp >= 60)
                angerBad += 0;
            else if (battle.playerHp >= 25)
                angerBad += 1;
            else
                angerBad += 2;

            nextLevel = LEVEL4;
            level4Unlocked = 1;
            level3Initialized = false;
            return ELEVATOR;
        }
    }
    else
    {
        DrawText("Press ENTER to try the battle again", GetScreenWidth()/2 - 215, 40, 28, YELLOW);

        if (IsKeyPressed(KEY_ENTER))
        {
            StartBattle();
            level3Phase = LEVEL3_PHASE_RPG;
        }
    }

    return LEVEL3;
}
