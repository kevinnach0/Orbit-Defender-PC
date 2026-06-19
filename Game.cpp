#include "Game.hpp"
#include <cmath>

// ---------------------------------------------------------------------------
void AttackPlayer::updateDamageSystem(float dt, bool userIsDragging) {
    // If the user is dragging, reset the attack timer.
    if (userIsDragging) {
        attackTimer  = attackDuration;
        isAttacking  = true;
    }

    // Count down the attack window.
    if (isAttacking) {
        attackTimer -= dt;
        if (attackTimer <= 0.0f) {
            isAttacking = false;
        }
    }

    // Count down the enemy's iframe window.
    if (isEnemyInHitState) {
        hitTimer -= dt;
        if (hitTimer <= 0.0f) {
            isEnemyInHitState = false;
        }
    }
}

// ---------------------------------------------------------------------------
bool AttackPlayer::checkHitboxCollision(float distanceToEnemy, int enemyIndex,
                                        const std::pair<float, float>& offsetFromEnemy) {
    if (distanceToEnemy < static_cast<float>(playerRadius) + 1.0f && isAttacking) {
        if (!isEnemyInHitState) {
            // First contact: activate iframes and register the hit.
            hitTimer          = hitStateDuration;
            isEnemyInHitState = true;
            hitTargets.insert({enemyIndex, offsetFromEnemy});
            return true;
        }
        // Already in iframes: update position for push, no new damage.
        hitTargets.insert_or_assign(enemyIndex, offsetFromEnemy);
    }
    return false;
}

// ---------------------------------------------------------------------------
void AttackPlayer::drawHitbox(Vector2 playerPos) const {
    if (isAttacking) {
        DrawCircleV(playerPos, static_cast<float>(playerRadius), {0, 0, 255, 100});
        DrawCircleLinesV(playerPos, static_cast<float>(playerRadius), BLUE);
    }
}
