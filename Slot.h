#pragma once

#include <vector>
#include "MyTypes.h"


struct Handle {
    u32 index;
    u32 generation;
};

template <typename T>
struct Slot {
    T data;
    u32 generation = 0;
};


template <typename T>
class SlotMap {

private:
    std::vector<Slot<T>> slots;
    std::vector<u32> freeIndices;
    T nullItem;

public:

    u32 GetSize() {
        return slots.size();
    }

    std::vector<Slot<T>>& GetVector() {
        return slots;
    }

    SlotMap(size_t reservedMemorySlots = 256) {
        slots.reserve(reservedMemorySlots);
    }

    Handle Insert(const T& item) {
        uint32_t index;

        // 1. If we have a deleted slot, reuse it!
        if (!freeIndices.empty()) {
            index = freeIndices.back();
            freeIndices.pop_back();

            slots[index].data = item;
            slots[index].active = true;
            slots[index].generation++; // Increment generation!
        }
        // 2. Otherwise, grow the vector dynamically
        else {
            index = static_cast<uint32_t>(slots.size());
            slots.push_back({item, 1, true});
        }

        return Handle{ index, slots[index].generation };
    }

    void Remove(Handle handle) {
        if (IsValid(handle)) {
            slots[handle.index].active = false;
            freeIndices.push_back(handle.index); // Add back to free list
        }
    }

    bool IsValid(Handle handle) const {
        if (handle.index >= slots.size())
            return false;
        const auto& slot = slots[handle.index];
        return slot.generation == handle.generation;
    }

    T* GetItemPtr(Handle handle) {
        if (IsValid(handle)) {
            return &slots[handle.index].data;
        }
        return nullptr;
    }

    T& GetItemRef(Handle handle) {
        if (IsValid(handle)) {
            return slots[handle.index].data;
        }
        return nullItem;
    }

    bool IsNullItem(T& item) {
        return item == nullItem;
    }


    std::vector<Slot<T>>& GetRawSlots() {
        return slots;
    }



    Handle GetFreeHandle() {
        u32 index;
        // Try reuse existing
        if (!freeIndices.empty()) {
            index = freeIndices.back();
            freeIndices.pop_back();
            Slot<T>& slot = slots[index];
            slot.generation = slot.generation + 1;
            slot.data = T();
        }
        // Create a new object
        else {
            u32 generation = 1;
            index = static_cast<u32>(slots.size());
            Slot<T> item = { T(), generation};
            slots.push_back(item);
        }
        Slot<T>& outSlot = slots[index];
        return { index, outSlot.generation };
    }


};