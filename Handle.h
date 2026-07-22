#pragma once
#include "MyTypes.h"


struct Handle {
    u32 index;
    u32 generation;

    bool IsEmpty() const{
        return index == 0 && generation == 0;
    }

    void Copy(const Handle& other) {
        index = other.index;
        generation = other.generation;
    }

    // Added equality operators so handles can be compared easily
    bool operator==(const Handle& other) const {
        return index == other.index && generation == other.generation;
    }
    bool operator!=(const Handle& other) const {
        return !(*this == other);
    }
};

/// Alias to 'Handle'
using ShaderHandle = Handle;
/// Alias to 'Handle'
using MaterialHandle = Handle;
/// Alias to 'Handle'
using MeshHandle = Handle;
/// Alias to 'Handle'
using TransformHandle = Handle;
/// Alias to 'Handle'
using ObjectHandle = Handle;
/// Alias to 'Handle'
using TextureHandle = Handle;