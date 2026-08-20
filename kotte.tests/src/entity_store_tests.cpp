#include "kotte/entity_store.hpp"

#include <cstdio>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
    void require(bool condition, std::string_view message){
        if(!condition){
            throw std::runtime_error{std::string{message}};
        }
    }

    void stale_handle_cannot_access_a_reused_slot(){
        kotte::EntityStore store;
        store.reserve(1);

        const kotte::EntityHandle crate_handle = store.create({kotte::EntityKind::crate});
        require(store.contains(crate_handle), "A newly created handle must resolve.");
        require(store.live_count() == 1, "Creating an entity must increase the live count.");

        const kotte::EntityHandle wrong_generation{
            crate_handle.slot,
            static_cast<std::uint32_t>(crate_handle.generation + 1)
        };
        require(!store.contains(wrong_generation), "An occupied slot must reject the wrong generation.");
        require(!store.contains({}), "A default out-of-range handle must be rejected.");

        require(store.destroy(crate_handle), "A live handle must be destroyable.");
        require(!store.contains(crate_handle), "A destroyed handle must stop resolving immediately.");
        require(!store.contains(wrong_generation), "An empty slot must not resolve even with its current generation.");
        require(!store.destroy(crate_handle), "Repeated destruction must be harmless.");

        const kotte::EntityHandle enemy_handle = store.create({kotte::EntityKind::enemy});
        require(enemy_handle.slot == crate_handle.slot, "Creation should reuse the available slot.");
        require(enemy_handle.generation != crate_handle.generation, "Slot reuse must advance the generation.");
        require(!store.contains(crate_handle), "A stale handle must not resolve to the replacement entity.");

        const kotte::Entity* enemy = store.try_get(enemy_handle);
        require(enemy != nullptr && enemy->kind == kotte::EntityKind::enemy,
            "The replacement handle must resolve to the replacement entity.");
        require(store.live_count() == 1, "Slot reuse must preserve the correct live count.");
        require(store.slot_count() == 1, "Slot reuse must not grow storage.");
    }

    void handles_survive_storage_reallocation(){
        kotte::EntityStore store;
        store.reserve(1);

        const kotte::EntityHandle player_handle = store.create({kotte::EntityKind::player});
        const kotte::EntityHandle crate_handle = store.create({kotte::EntityKind::crate});

        require(store.contains(player_handle), "Growing slot storage must not invalidate an existing handle.");
        require(store.contains(crate_handle), "The handle created after storage growth must resolve.");
    }

    void relationships_do_not_keep_stale_owners_alive(){
        kotte::EntityStore store;

        const kotte::EntityHandle player_handle = store.create({kotte::EntityKind::player});
        const kotte::EntityHandle bomb_handle = store.create({
            .kind = kotte::EntityKind::bomb,
            .bomb_state = kotte::BombState{
                .owner = player_handle,
                .fuse_remaining = 2.0f,
                .blast_range = 3
            }
        });

        require(store.destroy(player_handle), "The relationship must not own the player lifetime.");
        const kotte::EntityHandle replacement = store.create({kotte::EntityKind::enemy});

        const kotte::Entity* bomb = store.try_get(bomb_handle);
        if(bomb == nullptr || !bomb->bomb_state.has_value()){
            throw std::runtime_error{"The bomb must retain its relationship value."};
        }

        const kotte::EntityHandle retained_owner = bomb->bomb_state->owner;
        require(retained_owner == player_handle, "The bomb must still identify its original owner.");
        require(!store.contains(retained_owner), "A relationship must validate its handle before use.");
        require(replacement.slot == player_handle.slot, "The test must exercise owner-slot reuse.");
    }
}

int main(){
    try{
        stale_handle_cannot_access_a_reused_slot();
        handles_survive_storage_reallocation();
        relationships_do_not_keep_stale_owners_alive();
        std::println("EntityStore tests passed.");
        return 0;
    } catch(const std::exception& error){
        std::println(stderr, "EntityStore tests failed: {}", error.what());
        return 1;
    }
}
