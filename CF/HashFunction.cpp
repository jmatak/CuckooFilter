#include "HashFunction.h"


HashFunction::HashFunction() {
    ::std::random_device random;
    for (auto v : {&multiply_, &add_}) {
        *v = random();
    }
}

uint32_t HashFunction::hash(uint32_t key) const {
    return (add_ + multiply_ * static_cast<decltype(multiply_)>(key));
}

uint32_t HashFunction::hash(const std::string &key) const {
    const char *s = key.c_str();
    uint32_t h = FIRSTH;
    while (*s) {
        h = (h * HASH_A) ^ (s[0] * HASH_B);
        s++;
    }
    return h;
}

