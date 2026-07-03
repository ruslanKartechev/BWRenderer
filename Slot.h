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
    bool active;
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
        printf("null item return!\n");
        return nullItem;
    }

    bool IsNullItem(T& item) {
        return item == nullItem;
    }


    std::vector<Slot<T>>& GetRawSlots() {
        return slots;
    }

    /// Will either find a vacant unused object inside internal buffer or extend the buffer and create a new object.
    /// Returns a handle to a slot in the internal buffer
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


    /// Will either find a vacant unused object inside internal buffer or extend the buffer and create a new object.
    /// Returns a reference to an object inside internal buffer and a handle as out param.
    T& MakeNew(Handle& outHandle) {
        outHandle = GetFreeHandle();
        return slots[outHandle.index].data;
        // uint32_t index;
        // // REUSE a slot if it has been deleted
        // if (!freeIndices.empty()) {
        //     index = freeIndices.back();
        //     freeIndices.pop_back();
        //     auto& slot = slots[index];
        //     slot.active = true;
        //     slot.generation += 1; // Increment generation!
        //     outHandle = Handle{ index, slots[index].generation };
        //     return slot.data;
        // }
        // // Extend vector if no slot is vacant
        // index = static_cast<uint32_t>(slots.size());
        // slots.resize(index + 4);
        // auto& slot = slots[index];
        // slot.active = true;
        // outHandle = Handle{ index, slots[index].generation };
        // return slot.data;
    }

};