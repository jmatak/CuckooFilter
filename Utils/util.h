#include <stdint.h>
#include <stdlib.h>

struct Victim {
    uint32_t fp = 0;
    size_t index = 0;
};

static const size_t highestPowerOfTwo(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    v >>= 1;
    return v;
}
