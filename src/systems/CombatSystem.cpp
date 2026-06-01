#include "CombatSystem.hpp"
#include "entities/Player.hpp"
#include "entities/Mob.hpp"
#include "items/Tool.hpp"
#include "systems/ParticleSystem.hpp"
#include "core/SoundManager.hpp"
#include <random>
#include <cmath>

void CombatSystem::checkSwordHit(Player& player,
                                  std::vector<std::unique_ptr<Mob>>& mobs,
                                  ParticleSystem& particles) {
    if (!player.isSwinging()) return;

    auto* sel = player.getInventory().getSelectedSlot();
    TileId held = (sel && sel->count > 0) ? sel->tileId : TileId::Air;
    if (!isSword(held)) return;

    Rectangle hitbox = player.getSwingHitbox();
    if (hitbox.width <= 0 || hitbox.height <= 0) return;

    Vector2 playerCenter = {
        player.getPosition().x + player.getBounds().width / 2,
        player.getPosition().y + player.getBounds().height / 2
    };

    for (auto& mob : mobs) {
        if (!CheckCollisionRecs(hitbox, mob->getBounds())) continue;

        int damage = 20;
        auto* selItem = player.getInventory().getSelectedSlot();
        if (selItem) {
            TileId heldId = selItem->tileId;
            if (heldId == TileId::Pickaxe || heldId == TileId::Axe || isHammer(heldId)) damage = 5;
        }

        mob->takeDamage(damage);
        SoundManager::instance().play(SoundManager::SwordHit);

        float px = mob->getBounds().x + mob->getBounds().width / 2;
        float py = mob->getBounds().y + mob->getBounds().height / 2;
        for (int i = 0; i < 5; ++i) {
            particles.emit({px, py}, {0, -100}, {255, 50, 50, 255}, 0.4f, 3, 1);
        }

        Vector2 mobCenter{
            mob->getBounds().x + mob->getBounds().width / 2,
            mob->getBounds().y + mob->getBounds().height / 2
        };
        Vector2 dir{mobCenter.x - playerCenter.x, mobCenter.y - playerCenter.y};
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f) { dir.x /= len; dir.y /= len; }
        mob->knockback(dir);
    }
}

void CombatSystem::checkMobContactDamage(const std::vector<std::unique_ptr<Mob>>& mobs,
                                          Player& player) {
    if (player.getHealth() <= 0) return;

    for (const auto& mob : mobs) {
        if (CheckCollisionRecs(player.getBounds(), mob->getBounds())) {
            player.takeDamage(mob->getContactDamage());
        }
    }
}
