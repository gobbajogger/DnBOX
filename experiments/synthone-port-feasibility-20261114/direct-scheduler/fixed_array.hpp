// Minimal caller-owned array for the freestanding target C++ profile.
// Not a standard-library replacement; only the scheduler's required operations.
#pragma once
#include <stddef.h>
namespace s1scheduler {
template<class T,size_t N> struct FixedArray {
    T elements[N]{};
    T &operator[](size_t i) { return elements[i]; }
    const T &operator[](size_t i) const { return elements[i]; }
    T *begin() { return elements; }
    const T *begin() const { return elements; }
    T *end() { return elements+N; }
    const T *end() const { return elements+N; }
};
} // namespace s1scheduler
