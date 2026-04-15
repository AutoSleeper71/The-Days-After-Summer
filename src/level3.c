/* Level 3 scene logic.
   Starts with dialogue, then switches into a simple turn-based battle, then resolves the result. */

#include "level3.h"
#include "raylib.h"
#include "game.h"
#include "dialog.h"
#include "events.h"
#include "dialog_scripts.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define PLAYER_MAX_HP 100
#define ENEMY_MAX_HP 120
// File-specific compile-time limit used to size arrays safely.
#define MAX_LEVEL3_NODES 96
#define LEVEL3_START_FADE_TIME 2.0f

#define PLAYER_BAR_W 240
#define ENEMY_BAR_W 240

static Music battleMusic;
static bool musicLoaded = false;
static bool musicPlaying = false;

typedef enum
{
    PHASE_DIALOG,
    PHASE_BATTLE,
    PHASE_RESULT
} Level3Phase;

typedef enum
{
    TURN_PLAYER,
    TURN_ENEMY_DELAY,
    TURN_ENEMY_ATTACK
} BattleTurn;

/* Stores all battle-related values in one place so the fight can be reset and updated cleanly. */
typedef struct
{
    int playerHp;
    int enemyHp;
    int guard;
    int healsLeft;
    int healsUsed;

    bool finished;
    bool playerWon;

    BattleTurn turn;
    float timer;

    float shownPlayerHp;
    float shownEnemyHp;

    float playerHitTimer;
    float enemyHitTimer;
    float playerAttackTimer;
    float enemyAttackTimer;
    float healFlashTimer;
    float screenShakeTimer;
    float reportFlashTimer;

    char message[256];
} BattleState;

static Level3Phase phase;
static BattleState battle;

static bool initialized = false;
static bool waitingEvent = false;
static bool introFadeActive = false;
static float introFadeTimer = 0.0f;

static DialogState dialog;
static DialogNode nodes[MAX_LEVEL3_NODES];

static Texture2D bg;
static Texture2D playerNeutral;
static Texture2D playerSad;
static Texture2D enemyDevil;

static bool loaded = false;

/* Small math helper: clamps a float to the 0..1 range. */
static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

/* Smoothly move one float toward another.
   Used for HP bar animation and attack/hit timers. */
static float ApproachFloat(float current, float target, float speed)
{
    if (current < target)
    {
        current += speed;
        if (current > target) current = target;
    }
    else if (current > target)
    {
        current -= speed;
        if (current < target) current = target;
    }
    return current;
}

/* Try to load a texture, but return an empty texture if the file is missing. */
static Texture2D SafeLoad(const char *path)
{
    if (FileExists(path)) return LoadTexture(path);
    return (Texture2D){0};
}

static Texture2D LoadFirstExisting(const char *paths[], int count)
{
    for (int i = 0; i < count; i++)
    {
        if (FileExists(paths[i]))
            return LoadTexture(paths[i]);
    }
    return (Texture2D){0};
}

static void LoadAssets(void)
{
    if (loaded) return;

    if (!musicLoaded && FileExists("assets/audio/battle.mp3"))
    {
        battleMusic = LoadMusicStream("assets/audio/battle.mp3");
        musicLoaded = true;
    }

    bg = SafeLoad("assets/background/EmptyDiner.png");
    if (!bg.id) bg = SafeLoad("assets/background/Diner.png");

    playerNeutral = SafeLoad("assets/avatar/character.png");
    if (!playerNeutral.id) playerNeutral = SafeLoad("assets/avatar/Neutral.png");

    playerSad = SafeLoad("assets/avatar/HappyMan.png");
    if (!playerSad.id) playerSad = SafeLoad("assets/avatar/Sad.png");
    if (!playerSad.id) playerSad = playerNeutral;

    const char *devilCandidates[] = {
        "assets/avatar/Character_demon.png",
        "assets/avatar/Character_devil.png",
        "assets/avatar/Devil.png",
        "assets/avatar/devil.png",
        "assets/avatar/DevilCharacter.png",
        "assets/avatar/Disapointed.png"
    };

    enemyDevil = LoadFirstExisting(devilCandidates, (int)(sizeof(devilCandidates) / sizeof(devilCandidates[0])));
    if (!enemyDevil.id) enemyDevil = playerSad.id ? playerSad : playerNeutral;

    loaded = true;
}



static void StartBattle(void)
{
    battle.playerHp = PLAYER_MAX_HP;
    battle.enemyHp = ENEMY_MAX_HP;
    battle.guard = 0;
    battle.healsLeft = 3;
    battle.healsUsed = 0;

    battle.finished = false;
    battle.playerWon = false;

    battle.turn = TURN_PLAYER;
    battle.timer = 0.0f;

    battle.shownPlayerHp = (float)PLAYER_MAX_HP;
    battle.shownEnemyHp = (float)ENEMY_MAX_HP;

    battle.playerHitTimer = 0.0f;
    battle.enemyHitTimer = 0.0f;
    battle.playerAttackTimer = 0.0f;
    battle.enemyAttackTimer = 0.0f;
    battle.healFlashTimer = 0.0f;
    battle.screenShakeTimer = 0.0f;
    battle.reportFlashTimer = 0.0f;

    strcpy(battle.message, "Battle starts. Endure the hits, heal, and strike back.");
}

static void InitLevel3State(void)
{
    LoadAssets();

    memcpy(nodes, LEVEL3_TEMPLATE, sizeof(DialogNode) * LEVEL3_TEMPLATE_COUNT);
    DialogStart(&dialog, nodes);

    EventsInit();
    EventsTrigger(EVENT_CHANGE_BACKGROUND, BG_INSIDE, AVATAR_NONE, SOUND_NONE, INSPECT_NONE);

    StartBattle();
    phase = PHASE_DIALOG;
    waitingEvent = false;
    introFadeActive = true;
    introFadeTimer = 0.0f;

    initialized = true;
}

void InitLevel3(void)
{
    initialized = false;
}

/* Let temporary animation timers decay back to zero every frame. */
static void StepBattleAnimationTimers(float dt)
{
    battle.playerHitTimer = ApproachFloat(battle.playerHitTimer, 0.0f, dt);
    battle.enemyHitTimer = ApproachFloat(battle.enemyHitTimer, 0.0f, dt);
    battle.playerAttackTimer = ApproachFloat(battle.playerAttackTimer, 0.0f, dt);
    battle.enemyAttackTimer = ApproachFloat(battle.enemyAttackTimer, 0.0f, dt);
    battle.healFlashTimer = ApproachFloat(battle.healFlashTimer, 0.0f, dt);
    battle.screenShakeTimer = ApproachFloat(battle.screenShakeTimer, 0.0f, dt);
    battle.reportFlashTimer = ApproachFloat(battle.reportFlashTimer, 0.0f, dt * 1.5f);

    battle.shownPlayerHp = ApproachFloat(battle.shownPlayerHp, (float)battle.playerHp, 45.0f * dt);
    battle.shownEnemyHp = ApproachFloat(battle.shownEnemyHp, (float)battle.enemyHp, 45.0f * dt);
}

/* Trigger short visual feedback after someone gets hit. */
static void PushBattleFeedback(bool playerWasHit, bool enemyWasHit)
{
    if (playerWasHit) battle.playerHitTimer = 0.22f;
    if (enemyWasHit) battle.enemyHitTimer = 0.22f;
    battle.screenShakeTimer = 0.18f;
    battle.reportFlashTimer = 0.20f;
}

static void UpdateBattle(void)
{
    float dt = GetFrameTime();
    StepBattleAnimationTimers(dt);

    if (battle.finished) return;

    if (battle.turn == TURN_PLAYER)
    {
        // Attack: basic damage option. It becomes slightly stronger after several heals so the fight cannot stall forever.
        if (IsKeyPressed(KEY_A))
        {
            int dealt = GetRandomValue(11, 15);
            if (battle.healsUsed >= 3) dealt += GetRandomValue(3, 4);

            battle.enemyHp -= dealt;
            if (battle.enemyHp < 0) battle.enemyHp = 0;

            battle.playerAttackTimer = 0.18f;
            PushBattleFeedback(false, true);

            

            if (battle.enemyHp <= 0)
            {
                battle.finished = true;
                battle.playerWon = true;
                phase = PHASE_RESULT;
                return;
            }

            battle.turn = TURN_ENEMY_DELAY;
            battle.timer = 0.0f;
        }
        // Guard: reduce the next incoming hit.
        else if (IsKeyPressed(KEY_G))
        {
            battle.guard = 1;
            strcpy(battle.message, "Player guards and braces for the next hit.");
            battle.reportFlashTimer = 0.18f;
            battle.turn = TURN_ENEMY_DELAY;
            battle.timer = 0.0f;
        }
        // Heal: restore HP, but with limited uses.
        else if (IsKeyPressed(KEY_H))
        {
            if (battle.healsLeft > 0)
            {
                int healed = GetRandomValue(28, 34);
                battle.playerHp += healed;
                if (battle.playerHp > PLAYER_MAX_HP) battle.playerHp = PLAYER_MAX_HP;

                battle.healsLeft--;
                battle.healsUsed++;
                battle.healFlashTimer = 0.25f;
                battle.reportFlashTimer = 0.20f;

                sprintf(battle.message,
                        "Player healed %d HP. Heals left: %d.",
                        healed, battle.healsLeft);

                battle.turn = TURN_ENEMY_DELAY;
                battle.timer = 0.0f;
            }
            else
            {
                strcpy(battle.message, "No heals left.");
                battle.reportFlashTimer = 0.18f;
            }
        }
    }
    else if (battle.turn == TURN_ENEMY_DELAY)
    {
        battle.timer += dt;
        if (battle.timer > 0.55f)
            battle.turn = TURN_ENEMY_ATTACK;
    }
    else if (battle.turn == TURN_ENEMY_ATTACK)
    {
        int taken = GetRandomValue(10, 14);
if (battle.guard)
{
    taken = GetRandomValue(4, 7);
    battle.guard = 0;
            sprintf(battle.message,
                    "Enemy struck for %d damage, but the guard softened the blow.",
                    taken);
        }
        else
        {
            sprintf(battle.message,
                    "Enemy dealt %d damage and pressed the attack.",
                    taken);
        }

        battle.enemyAttackTimer = 0.18f;
        PushBattleFeedback(true, false);

        battle.playerHp -= taken;
        if (battle.playerHp < 0) battle.playerHp = 0;

        if (battle.playerHp <= 0)
        {
            battle.finished = true;
            battle.playerWon = false;
            phase = PHASE_RESULT;
            return;
        }

        battle.turn = TURN_PLAYER;
    }
}

/* Draw one combat HP bar and show whose turn it is with an arrow. */
static void DrawHealthBar(float x, float y, float width, float height, int hpShown, int hpMax, const char *label, bool activeTurn)
{
    float ratio = (hpMax > 0) ? ((float)hpShown / (float)hpMax) : 0.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    Rectangle outer = { x, y, width, height };
    DrawRectangleRounded(outer, 0.18f, 8, Fade(BLACK, 0.76f));
    DrawRectangleRoundedLinesEx(outer, 0.18f, 8, activeTurn ? 3.0f : 2.0f, WHITE);
    DrawRectangle((int)x + 8, (int)y + 8, (int)width - 16, (int)height - 16, Fade(DARKGRAY, 0.95f));
    DrawRectangle((int)x + 8, (int)y + 8, (int)((width - 16.0f) * ratio), (int)height - 16, RED);

    DrawText(label, (int)x + 12, (int)y + 10, 22, WHITE);
    DrawText(TextFormat("%d / %d", hpShown, hpMax), (int)(x + width - 120), (int)y + 10, 22, WHITE);

    if (activeTurn)
    {
        Vector2 a = { x + width * 0.5f, y - 18.0f };
        Vector2 b = { x + width * 0.5f - 14.0f, y - 42.0f };
        Vector2 c = { x + width * 0.5f + 14.0f, y - 42.0f };
        DrawTriangle(a, b, c, YELLOW);
    }
}

/* Draw the simple command menu for the player turn. */
static void DrawChoiceBox(int w, int h)
{
    Rectangle box = { w/2.0f - 210.0f, h/2.0f + 70.0f, 420.0f, 170.0f };
    DrawRectangleRounded(box, 0.12f, 10, Fade(BLACK, 0.78f));
    DrawRectangleRoundedLinesEx(box, 0.12f, 10, 2.0f, WHITE);

    DrawText("Choose Move", (int)box.x + 120, (int)box.y + 10, 30, WHITE);

    Rectangle attack = { box.x + 18, box.y + 52, box.width - 36, 34 };
    Rectangle guard  = { box.x + 18, box.y + 92, box.width - 36, 28 };
    Rectangle heal   = { box.x + 18, box.y + 126, box.width - 36, 28 };

    DrawRectangleLinesEx(attack, 1.5f, WHITE);
    DrawRectangleLinesEx(guard, 1.5f, WHITE);
    DrawRectangleLinesEx(heal, 1.5f, WHITE);

    DrawText("A - Attack", (int)attack.x + 14, (int)attack.y + 5, 24, WHITE);
    DrawText("G - Guard",  (int)guard.x + 14, (int)guard.y + 2, 22, WHITE);
    DrawText(TextFormat("H - Heal   (%d left)", battle.healsLeft), (int)heal.x + 14, (int)heal.y + 2, 22, WHITE);
}

static void DrawWrappedReport(const char *text, int x, int y, int maxWidth, int fontSize)
{
    Font font = GetFontDefault();
    int len = (int)strlen(text);
    int start = 0;
    char line[256];

    while (start < len)
    {
        int end = start;
        int lastSpace = -1;

        while (end < len)
        {
            if (text[end] == ' ') lastSpace = end;

            int count = end - start + 1;
            if (count >= 255) break;

            strncpy(line, &text[start], count);
            line[count] = '\0';

            if (MeasureTextEx(font, line, (float)fontSize, 1.0f).x > (float)maxWidth)
            {
                if (lastSpace != -1 && lastSpace > start) end = lastSpace;
                break;
            }
            end++;
        }

        int count = end - start;
        if (count <= 0) count = 1;

        strncpy(line, &text[start], count);
        line[count] = '\0';

        DrawText(line, x, y, fontSize, WHITE);
        y += fontSize + 4;

        start = end;
        if (text[start] == ' ') start++;
    }
}

/* Draw a character sprite with optional attack/hit movement and highlight. */
static void DrawFighter(Texture2D tex, float x, float y, float scale, bool flip, float attackTimer, float hitTimer, bool highlight)
{
    if (!tex.id) return;

    float width = tex.width * scale;
    float height = tex.height * scale;
    float attackPush = 0.0f;

    if (attackTimer > 0.0f)
    {
        float t = Clamp01(attackTimer / 0.18f);
        attackPush = 28.0f * t;
    }

    if (flip) x += attackPush;
    else x -= attackPush;

    if (hitTimer > 0.0f)
        x += (GetRandomValue(-4, 4));

    Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
    Rectangle dst = { x, y, flip ? -width : width, height };

    if (highlight)
        DrawCircle((int)(x + (flip ? -width * 0.5f : width * 0.5f)), (int)(y + height * 0.52f), (int)(height * 0.33f), Fade(YELLOW, 0.18f));

    DrawTexturePro(tex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);

    if (hitTimer > 0.0f)
    {
        DrawRectangle((int)(x + (flip ? -width : 0.0f)), (int)y, (int)width, (int)height, Fade(WHITE, hitTimer * 0.45f));
        DrawCircleLines((int)(x + (flip ? -width * 0.5f : width * 0.5f)), (int)(y + height * 0.45f), height * 0.24f, Fade(WHITE, hitTimer));
        DrawCircleLines((int)(x + (flip ? -width * 0.5f : width * 0.5f)), (int)(y + height * 0.45f), height * 0.30f, Fade(WHITE, hitTimer * 0.7f));
    }
}

static void DrawBattle(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if (bg.id)
    {
        DrawTexturePro(bg,
            (Rectangle){0, 0, (float)bg.width, (float)bg.height},
            (Rectangle){0, 0, (float)w, (float)h},
            (Vector2){0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangle(0, 0, w, h, DARKGRAY);
    }

    if (battle.screenShakeTimer > 0.0f)
    {
        DrawRectangle(0, 0, w, h, Fade(WHITE, battle.screenShakeTimer * 0.18f));
    }

    DrawText("LEVEL 3 - ANGER", 50, 30, 34, WHITE);

    bool playerTurn = (battle.turn == TURN_PLAYER) && !battle.finished;
    bool enemyTurn = (battle.turn != TURN_PLAYER) && !battle.finished;

    DrawHealthBar(55.0f, 72.0f, PLAYER_BAR_W, 48.0f, (int)(battle.shownPlayerHp + 0.5f), PLAYER_MAX_HP, "Player HP", playerTurn);
    DrawHealthBar((float)w - ENEMY_BAR_W - 55.0f, 72.0f, ENEMY_BAR_W, 48.0f, (int)(battle.shownEnemyHp + 0.5f), ENEMY_MAX_HP, "Enemy HP", enemyTurn);

    Rectangle reportBox = { w/2.0f - 250.0f, 58.0f, 500.0f, 96.0f };
    DrawRectangleRounded(reportBox, 0.10f, 10, Fade(BLACK, 0.78f));
    DrawRectangleRoundedLinesEx(reportBox, 0.10f, 10, 2.0f, WHITE);
    if (battle.reportFlashTimer > 0.0f)
        DrawRectangleRounded(reportBox, 0.10f, 10, Fade(WHITE, battle.reportFlashTimer * 0.12f));
    DrawText("Battle Report", (int)reportBox.x + 16, (int)reportBox.y + 8, 24, WHITE);
    DrawWrappedReport(battle.message, (int)reportBox.x + 16, (int)reportBox.y + 38, (int)reportBox.width - 32, 20);

    Texture2D playerTex = (battle.playerHp <= 35) ? playerSad : playerNeutral;
    float playerScale = 0.5f;
float enemyScale  = 0.42f;

float playerX = w * 0.18f;
float playerY = h * 0.33f;

float enemyX = w * 0.68f;
float enemyY = h * 0.33f;

    DrawFighter(playerTex, playerX, playerY, playerScale, true,
                battle.playerAttackTimer, battle.playerHitTimer,
                playerTurn || battle.healFlashTimer > 0.0f);

    DrawFighter(enemyDevil, enemyX, enemyY, enemyScale, false,
                battle.enemyAttackTimer, battle.enemyHitTimer,
                enemyTurn);

    if (battle.healFlashTimer > 0.0f)
    {
        DrawCircle((int)(w * 0.22f), (int)(h * 0.53f), 70, Fade(GREEN, battle.healFlashTimer * 0.22f));
        DrawText("+", (int)(w * 0.22f) - 10, (int)(h * 0.53f) - 30, 60, Fade(WHITE, battle.healFlashTimer));
    }

    if (phase != PHASE_RESULT)
        DrawChoiceBox(w, h);
    else
    {
        Rectangle resultBox = { w/2.0f - 220.0f, h/2.0f + 90.0f, 440.0f, 88.0f };
        DrawRectangleRounded(resultBox, 0.12f, 10, Fade(BLACK, 0.80f));
        DrawRectangleRoundedLinesEx(resultBox, 0.12f, 10, 2.0f, WHITE);
        DrawText(battle.playerWon ? "You won the fight." : "You lost the fight.", (int)resultBox.x + 95, (int)resultBox.y + 18, 28, WHITE);
        DrawText("ENTER -> next", (int)resultBox.x + 125, (int)resultBox.y + 50, 24, YELLOW);
    }
}

static void DrawIntroFadeOverlay(void)
{
    if (!introFadeActive) return;

    float alpha = 1.0f - (introFadeTimer / LEVEL3_START_FADE_TIME);
    alpha = Clamp01(alpha);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
}

GameState UpdateLevel3(void)
{
    if (!initialized)
        InitLevel3State();

    if (IsKeyPressed(KEY_ESCAPE) && !IsSettingsMenuOpen())
    {
        OpenPauseMenu();
        return LEVEL3;
    }

    if (phase == PHASE_DIALOG)
    {
        Texture2D *bgTex = EventsGetCurrentBackground();
        if (bgTex && bgTex->id)
            DrawTexture(*bgTex, 0, 0, WHITE);
        else
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

        if (IsSettingsMenuOpen())
        {
            EventsDrawOverlay();
            DrawIntroFadeOverlay();

            {
                SettingsResult settings = UpdateAndDrawSettingsMenu();
                if (settings == SETTINGS_RESULT_GO_TO_MENU)
                {
                    if (musicPlaying)
                    {
                        StopMusicStream(battleMusic);
                        musicPlaying = false;
                    }
                    SaveGameForState(LEVEL3);
                    return MENU;
                }
                if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
            }
            return LEVEL3;
        }

        EventsUpdate();

        if (introFadeActive)
        {
            introFadeTimer += GetFrameTime();
            if (introFadeTimer >= LEVEL3_START_FADE_TIME)
            {
                introFadeTimer = LEVEL3_START_FADE_TIME;
                introFadeActive = false;
            }
        }

        if (!introFadeActive)
        {
            if (waitingEvent && !dialog.finished && !EventsBusy())
            {
                waitingEvent = false;
                DialogResume(&dialog);
            }

            if (!dialog.finished)
            {
                if (!waitingEvent)
                {
                    DialogEvent ev = DialogUpdate(&dialog);
                    if (ev != EVENT_NONE)
                    {
                        DialogNode *n = &dialog.nodes[dialog.index];
                        EventsTrigger(ev, n->backgroundId, n->avatarId, n->soundId, n->inspectId);
                        waitingEvent = true;
                    }
                }

                if (EventsIsDialogVisible())
                    DialogDraw(&dialog);
            }
            else
            {
                phase = PHASE_BATTLE;
                if (musicLoaded && !musicPlaying)
                {
                    PlayMusicStream(battleMusic);
                    musicPlaying = true;
                }
            }
        }

        EventsDrawOverlay();
        DrawIntroFadeOverlay();
        return LEVEL3;
    }

    if (phase == PHASE_BATTLE)
    {
        DrawBattle();

        if (IsSettingsMenuOpen())
        {
            SettingsResult settings = UpdateAndDrawSettingsMenu();
            if (settings == SETTINGS_RESULT_GO_TO_MENU)
            {
                if (musicPlaying)
                {
                    StopMusicStream(battleMusic);
                    musicPlaying = false;
                }
                SaveGameForState(LEVEL3);
                return MENU;
            }
            if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
            return LEVEL3;
        }

        if (musicPlaying)
            UpdateMusicStream(battleMusic);

        UpdateBattle();
        return LEVEL3;
    }

    DrawBattle();

    if (IsSettingsMenuOpen())
    {
        SettingsResult settings = UpdateAndDrawSettingsMenu();
        if (settings == SETTINGS_RESULT_GO_TO_MENU)
        {
            if (musicPlaying)
            {
                StopMusicStream(battleMusic);
                musicPlaying = false;
            }
            SaveGameForState(LEVEL3);
            return MENU;
        }
        if (settings == SETTINGS_RESULT_EXIT) return GAME_EXIT;
        return LEVEL3;
    }

    if (musicPlaying)
    {
        UpdateMusicStream(battleMusic);
        if (battle.shownPlayerHp == (float)battle.playerHp && battle.shownEnemyHp == (float)battle.enemyHp)
        {
            StopMusicStream(battleMusic);
            musicPlaying = false;
        }
    }

    UpdateBattle();

    if (IsKeyPressed(KEY_ENTER))
{
    angerBad = battle.playerWon ? 1 : 0;
    initialized = false;
    return LEVEL4;
}

    return LEVEL3;
}
