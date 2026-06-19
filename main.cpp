#include "raylib.h"
#include "Game.hpp"
#include <cmath>
#include <vector>
#include <utility>

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------

/// Float clamp — equivalent to std::clamp without needing <algorithm>.
static float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/// Returns a small staggered offset so enemies don't stack on the exact same path.
/// Indices 0-3 produce a minor spread; anything beyond returns 0.
static float EnemySpreadOffset(int index) {
    if (index < 1)  return 0.1f;
    if (index <= 3) return static_cast<float>(index) / 10.0f;
    return 0.0f;
}

/// Returns whether an enemy is allowed to move this frame.
/// - false while the game is resetting.
/// - false on the exact frame the enemy was hit by the orbital hitbox.
static bool EnemyCanMove(bool isResetting, bool justGotHit) {
    if (isResetting) return false;
    if (justGotHit)  return false;
    return true;
}

// ---------------------------------------------------------------------------
//  Data types
// ---------------------------------------------------------------------------

struct Enemy {
    Vector2 pos    = {0.0f, 0.0f};
    Vector2 size   = {120.0f, 98.0f};
    Color   color  = RED;
    float   damage = 23.5f;   ///< Contact damage dealt to the player per hit.
};

// ---------------------------------------------------------------------------
//  Game constants
// ---------------------------------------------------------------------------

static constexpr int   ENEMY_COUNT       = 5;
static constexpr float PLAYER_RADIUS     = 38.0f;
static constexpr float ORBITAL_RADIUS    = 110.7f;
static constexpr float PLAYER_SPEED      = 578.0f;
static constexpr float ORBITAL_SPEED     = 6.5f;    // radians/sec for the satellite
static constexpr float MAX_PLAYER_LIFE   = 120.0f;
static constexpr float COLLISION_FLASH   = 1.3f;    // seconds the hit color is shown
static constexpr float CONTACT_DAMAGE_CD = 0.4f;    // seconds between contact damage ticks

// ---------------------------------------------------------------------------
int main()
{
    InitWindow(1280, 720, "Raylib Game");
    SetTargetFPS(60);

    // ---- Positions ---------------------------------------------------------
    Vector2 playerPos    = {0.0f, 0.0f};
    Vector2 orbitalPos   = {0.0f, 0.0f};   // satellite that doubles as the attack hitbox
    Vector2 mousePos     = {0.0f, 0.0f};
    float   orbitalAngle = 0.0f;

    // ---- Game state --------------------------------------------------------
    bool  needsReset      = true;    // true on startup and on player death
    bool  playerDragging  = false;

    float playerLife      = MAX_PLAYER_LIFE;
    Color playerColor     = WHITE;
    bool  playerTookHit   = false;
    float hitFlashTimer   = 0.0f;
    float contactDamageCd = 0.0f;   // prevents damage from firing every frame on contact

    // ---- Enemies -----------------------------------------------------------
    float enemySpeed = PLAYER_SPEED / 2.5f - 0.8f;

    std::vector<Enemy> enemies(ENEMY_COUNT);
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        enemies[i].pos = {120.0f + i * 240.0f, 350.0f};
    }

    // ---- Attack system -----------------------------------------------------
    AttackPlayer playerAttack(20);

    // ========================================================================
    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();

        // ---- Reset / initialization ----------------------------------------
        if (needsReset) {
            float w = static_cast<float>(GetScreenWidth());
            float h = static_cast<float>(GetScreenHeight());
            playerPos  = {w / 2.0f, h / 2.0f};
            orbitalPos = playerPos;
            needsReset = false;
        }

        // ---- Input: hold left mouse button to move -------------------------
        // On PC, holding left click replaces the touch drag from the mobile version.
        playerDragging = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

        if (playerDragging) {
            mousePos = GetMousePosition();
            float dx  = mousePos.x - playerPos.x;
            float dy  = mousePos.y - playerPos.y;
            float mag = sqrtf(dx * dx + dy * dy);

            if (mag > 5.0f) {
                playerPos.x += (dx / mag) * PLAYER_SPEED * dt;
                playerPos.y += (dy / mag) * PLAYER_SPEED * dt;
            }

            orbitalAngle += ORBITAL_SPEED * dt;
            orbitalPos = {
                playerPos.x + cosf(orbitalAngle) * ORBITAL_RADIUS,
                playerPos.y + sinf(orbitalAngle) * ORBITAL_RADIUS
            };
        }

        // ---- Attack system update ------------------------------------------
        playerAttack.updateDamageSystem(dt, playerDragging);

        // ---- Contact damage cooldown ---------------------------------------
        if (contactDamageCd > 0.0f) contactDamageCd -= dt;

        // ---- Enemy logic ---------------------------------------------------
        for (int i = 0; i < ENEMY_COUNT; ++i) {

            // -- Player body <-> enemy distance (body collision) -------------
            float clampX  = Clamp(playerPos.x, enemies[i].pos.x, enemies[i].pos.x + enemies[i].size.x);
            float clampY  = Clamp(playerPos.y, enemies[i].pos.y, enemies[i].pos.y + enemies[i].size.y);
            float bodyDx  = playerPos.x - clampX;
            float bodyDy  = playerPos.y - clampY;
            float bodyDist = sqrtf(bodyDx * bodyDx + bodyDy * bodyDy);

            // -- Satellite <-> enemy distance (attack hitbox) ----------------
            float sClampX   = Clamp(orbitalPos.x, enemies[i].pos.x, enemies[i].pos.x + enemies[i].size.x);
            float sClampY   = Clamp(orbitalPos.y, enemies[i].pos.y, enemies[i].pos.y + enemies[i].size.y);
            float sDx       = orbitalPos.x - sClampX;
            float sDy       = orbitalPos.y - sClampY;
            float orbitalDist = sqrtf(sDx * sDx + sDy * sDy);

            // -- Body collision: push player out and deal damage -------------
            const float BODY_SEPARATION = PLAYER_RADIUS;
            if (bodyDist < BODY_SEPARATION) {
                float overlap = BODY_SEPARATION - bodyDist;
                if (bodyDist > 0.0f) {
                    playerPos.x += (bodyDx / bodyDist) * overlap;
                    playerPos.y += (bodyDy / bodyDist) * overlap;
                } else {
                    playerPos.y -= overlap;
                }

                playerColor   = BROWN;
                playerTookHit = true;
                hitFlashTimer = COLLISION_FLASH;

                // Cooldown prevents damage from firing 60 times per second.
                if (contactDamageCd <= 0.0f) {
                    playerLife      -= enemies[i].damage;
                    contactDamageCd  = CONTACT_DAMAGE_CD;
                }
            }

            // -- Satellite collision: push satellite out (no player damage) --
            const float ORBITAL_SEP = 20.0f;
            if (orbitalDist < ORBITAL_SEP) {
                float overlap = ORBITAL_SEP - orbitalDist;
                if (orbitalDist > 0.0f) {
                    orbitalPos.x += (sDx / orbitalDist) * overlap * dt;
                    orbitalPos.y += (sDy / orbitalDist) * overlap * dt;
                } else {
                    orbitalPos.y -= overlap * dt;
                }
            }

            // -- Check attack hitbox -----------------------------------------
            std::pair<float, float> hitOffset = {sDx, sDy};
            bool justHit = playerAttack.checkHitboxCollision(orbitalDist, i, hitOffset);

            // -- Move enemy toward player ------------------------------------
            if (EnemyCanMove(false, justHit)) {
                float spread = EnemySpreadOffset(i);
                float eDx    = playerPos.x - enemies[i].pos.x;
                float eDy    = playerPos.y - enemies[i].pos.y;
                float eDist  = sqrtf(eDx * eDx + eDy * eDy);

                // Only move if not already overlapping the player.
                bool tooClose = (bodyDist < BODY_SEPARATION + 2.0f) ||
                                (orbitalDist < ORBITAL_SEP + 1.0f);
                if (!tooClose && eDist > 0.0f) {
                    enemies[i].pos.x += (eDx / eDist + spread) * enemySpeed * dt;
                    enemies[i].pos.y += (eDy / eDist + spread) * enemySpeed * dt;
                }
            }
        }

        // ---- Push hit enemies ----------------------------------------------
        playerAttack.pushObstacles(dt, enemies, orbitalPos);

        // ---- Enemy-enemy separation ----------------------------------------
        for (int i = 0; i < ENEMY_COUNT; ++i) {
            for (int j = i + 1; j < ENEMY_COUNT; ++j) {
                Rectangle ri = {enemies[i].pos.x, enemies[i].pos.y, enemies[i].size.x, enemies[i].size.y};
                Rectangle rj = {enemies[j].pos.x, enemies[j].pos.y, enemies[j].size.x, enemies[j].size.y};

                if (CheckCollisionRecs(ri, rj)) {
                    Rectangle overlap = GetCollisionRec(ri, rj);
                    if (overlap.width < overlap.height) {
                        float push = overlap.width * 0.5f;
                        if (enemies[j].pos.x < enemies[i].pos.x) {
                            enemies[j].pos.x -= push;
                            enemies[i].pos.x += push;
                        } else {
                            enemies[j].pos.x += push;
                            enemies[i].pos.x -= push;
                        }
                    } else {
                        float push = overlap.height * 0.5f;
                        if (enemies[j].pos.y < enemies[i].pos.y) {
                            enemies[j].pos.y -= push;
                            enemies[i].pos.y += push;
                        } else {
                            enemies[j].pos.y += push;
                            enemies[i].pos.y -= push;
                        }
                    }
                }
            }
        }

        // ---- Player death / reset ------------------------------------------
        if (playerLife <= 0.0f) {
            playerLife = MAX_PLAYER_LIFE;
            needsReset = true;
        }

        // ---- Hit flash timer -----------------------------------------------
        if (playerTookHit) {
            hitFlashTimer -= dt;
            if (hitFlashTimer <= 0.0f) {
                playerColor   = WHITE;
                playerTookHit = false;
            }
        }

        // ---- Render --------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Hold left click and drag to move", 34, 198, 33, BLUE);

        // Satellite (only visible while holding click)
        if (playerDragging) {
            DrawCircleV(orbitalPos, 20.0f, RED);
        }

        // Attack hitbox
        playerAttack.drawHitbox(orbitalPos);

        // Enemies
        for (int i = 0; i < ENEMY_COUNT; ++i) {
            DrawRectangleV(enemies[i].pos, enemies[i].size, enemies[i].color);
        }

        // Player
        DrawCircleV(playerPos, PLAYER_RADIUS, playerColor);

        // HUD: health
        DrawText(TextFormat("Health: %.0f", playerLife), 10, 10, 28, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
