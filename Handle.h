#ifndef RENDERGL_HANDLE_H
#define RENDERGL_HANDLE_H

struct Handle {
    u32 index = 0;
    u32 generation = 0;

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

#endif //RENDERGL_HANDLE_H
