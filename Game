#pragma once
#include "raylib.h"
#include <vector>
#include <map>
#include <utility>

// ---------------------------------------------------------------------------
//  AttackPlayer
//  Handles the player's attack system: orbital hitbox, enemy damage,
//  and the push effect when a hit connects.
// ---------------------------------------------------------------------------
class AttackPlayer {
public:
    // ---- Constructors ------------------------------------------------------
    AttackPlayer() = default;
    explicit AttackPlayer(int hitboxRadius) : playerRadius(hitboxRadius) {}

    // ---- Public API --------------------------------------------------------

    /// Updates attack and iframe timers. Call once per frame.
    void updateDamageSystem(float dt, bool userIsDragging);

    /// Checks if the orbital hitbox collided with enemy [index].
    /// Returns true only on the exact frame the first hit is registered.
    bool checkHitboxCollision(float distanceToEnemy, int enemyIndex,
                              const std::pair<float, float>& offsetFromEnemy);

    /// Pushes all enemies currently stored in hitTargets.
    template<typename T>
    void pushObstacles(float dt, std::vector<T>& obstacles, Vector2 orbitalCirclePos) {
        if (isAttacking && isEnemyInHitState && !hitTargets.empty()) {
            for (auto& [idx, _] : hitTargets) {
                float dx   = obstacles[idx].pos.x - orbitalCirclePos.x;
                float dy   = obstacles[idx].pos.y - orbitalCirclePos.y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 0.0f) {
                    obstacles[idx].pos.x += (dx / dist) * PUSH_FORCE * dt;
                    obstacles[idx].pos.y += (dy / dist) * PUSH_FORCE * dt;
                } else {
                    obstacles[idx].pos.y -= PUSH_FORCE * dt;
                }
            }
        } else {
            hitTargets.clear();
        }
    }

    /// Draws the attack hitbox if the player is currently attacking.
    void drawHitbox(Vector2 playerPos) const;

    // ---- Observable state (read-only from main) ----------------------------
    int  playerRadius = 20;
    bool isAttacking  = false;

private:
    // ---- Timers and state --------------------------------------------------
    float attackDuration    = 1.8f;   ///< How long the attack state lasts (seconds).
    float attackTimer       = 0.0f;
    float hitStateDuration  = 1.8f;   ///< Enemy iframe duration after being hit (seconds).
    float hitTimer          = 0.0f;
    bool  isEnemyInHitState = false;

    // ---- Constants ---------------------------------------------------------
    static constexpr float PUSH_FORCE = 688.0f;

    // ---- Enemies hit this attack window ------------------------------------
    /// Key = index in the obstacles vector, value = impact offset.
    std::map<int, std::pair<float, float>> hitTargets;
};
