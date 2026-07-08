#pragma once
#include <vector>
#include "MyTypes.h"
#include "Handle.h"


struct HandleSlot {
    u32 generation = 0;
    bool active = false;
};

template <typename T>
class SlotMap {
private:

public:
    std::vector<u32> freeIndices = {};
    T nullItem = T();


    std::vector<T> slots = {};
    std::vector<HandleSlot> handles = {};

    SlotMap(size_t reservedMemorySlots = 256) {
        slots.reserve(reservedMemorySlots);
        handles.reserve(reservedMemorySlots);
    }

    u32 GetSize() const {
        return static_cast<u32>(slots.size());
    }

    std::vector<T>& GetVector() {
        return slots;
    }

    [[nodiscard]] bool IsValid(Handle handle) const {
        if (handle.index >= slots.size())
            return false;
        const auto& slot = slots[handle.index];
        return handles[handle.index].active && handles[handle.index].generation == handle.generation;
    }

    void FreeHandle(Handle handle) {
        if (IsValid(handle)) {
            auto& hl = handles[handle.index];
            hl.active = false;
            hl.generation += 1;
            freeIndices.push_back(handle.index);
        }
    }

    T* GetItemPtr(Handle handle) {
        if (IsValid(handle)) {
            return &slots[handle.index];
        }
        return nullptr;
    }
    T& GetItemRef(Handle handle) {
        if (IsValid(handle)) {
            // printf(" [slot] [%d] returning data at %p  \n", handle.index, &slots[handle.index].data);
            return slots[handle.index];
        }
        return nullItem;
    }

    bool IsNullItem(const T& item) const {
        return &item == &nullItem; // Note: requires T to have operator==
    }

    /// Will either find a vacant unused object inside internal buffer or extend the buffer and create a new object.
    /// Returns a handle to a slot in the internal buffer
    Handle GetFreeHandle() {
        u32 index;
        int freeCount = freeIndices.size();
        if (!freeIndices.empty()) {

            index = freeIndices.back();
            freeIndices.pop_back();

            if (index >= 0 && index < slots.size()) {
                slots[index] = T();
                handles[index].active = true;
                return { index, handles[index].generation };
            }
        }

        index = static_cast<u32>(slots.size());
        slots.push_back(T());
        handles.push_back(HandleSlot());

        handles[index].active = true;
        handles[index].generation = 1;
        return { index, handles[index].generation };
    }

    T& GetNewObjectAndHandle(Handle& outHandle) {
        auto temp = GetFreeHandle();
        outHandle.Copy(temp);
        return slots[outHandle.index];
    }
};