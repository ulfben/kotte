#include "kotte/entity_store.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace kotte
{
    void EntityStore::reserve(std::size_t capacity){
        if(capacity > EntityHandle::invalid_slot){
            throw std::length_error{"EntityStore capacity exceeds the handle index range."};
        }

        slots_.reserve(capacity);
        free_slots_.reserve(capacity);
    }

    EntityHandle EntityStore::create(Entity entity){
        if(!free_slots_.empty()){
            const std::uint32_t slot_index = free_slots_.back();
            Slot& slot = slots_[slot_index];
            assert(!slot.entity.has_value());

            // Pop only after construction succeeds so a throwing Entity move
            // cannot silently lose a reusable slot.
            slot.entity.emplace(std::move(entity));
            free_slots_.pop_back();
            ++live_count_;
            return {slot_index, slot.generation};
        }

        if(slots_.size() >= EntityHandle::invalid_slot){
            throw std::length_error{"EntityStore has exhausted the handle index range."};
        }

        const auto slot_index = static_cast<std::uint32_t>(slots_.size());
        slots_.emplace_back(std::move(entity));
        ++live_count_;
        return {slot_index, first_generation_};
    }

    bool EntityStore::destroy(EntityHandle handle){
        if(!contains(handle)){
            return false;
        }

        Slot& slot = slots_[handle.slot];

        // Reserve the free-list entry before invalidating authoritative state.
        // If allocation fails, the entity and its handle remain unchanged.
        free_slots_.push_back(handle.slot);
        slot.entity.reset();
        slot.generation = next_generation(slot.generation);
        assert(live_count_ > 0);
        --live_count_;
        return true;
    }

    Entity* EntityStore::try_get(EntityHandle handle) noexcept{
        if(handle.slot >= slots_.size()){
            return nullptr;
        }

        Slot& slot = slots_[handle.slot];
        if(!slot.entity.has_value() || slot.generation != handle.generation){
            return nullptr;
        }

        return &*slot.entity;
    }

    const Entity* EntityStore::try_get(EntityHandle handle) const noexcept{
        if(handle.slot >= slots_.size()){
            return nullptr;
        }

        const Slot& slot = slots_[handle.slot];
        if(!slot.entity.has_value() || slot.generation != handle.generation){
            return nullptr;
        }

        return &*slot.entity;
    }

    bool EntityStore::contains(EntityHandle handle) const noexcept{
        return try_get(handle) != nullptr;
    }

    std::size_t EntityStore::live_count() const noexcept{
        return live_count_;
    }

    std::size_t EntityStore::slot_count() const noexcept{
        return slots_.size();
    }

    std::uint32_t EntityStore::next_generation(std::uint32_t generation) noexcept{
        ++generation;

        // The store never issues generation zero, keeping it available as an
        // obviously uninitialized value in diagnostics and tests.
        if(generation == 0){
            ++generation;
        }

        return generation;
    }
}
