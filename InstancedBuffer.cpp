#include "InstancedBuffer.h"


InstanceEntry& InstancedBuffer::AddNewOne() {
    entries.emplace_back();
    auto& item = entries.back();
    return item;
}