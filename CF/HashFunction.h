#ifndef CUCKOOFILTER_HASHFUNCTION_H
#define CUCKOOFILTER_HASHFUNCTION_H


#include <random>

#define HASH_A 54059
#define HASH_B 76963
#define FIRSTH 37
static const uint32_t MURMUR_CONST = 0x5bd1e995;

// Martin Dietzfelbinger, "Universal hashing and k-wise independent random
// variables via integer arithmetic without primes".
class HashFunction {
private:
    uint32_t multiply_, add_;

public:
    HashFunction();

    uint32_t hash(uint32_t key) const;

    uint32_t hash(const std::string &key) const;
};


inline static uint32_t fingerprintComplement(const size_t index, const uint32_t fp) {
    return index ^ (fp * MURMUR_CONST);
}

#endif //CUCKOOFILTER_HASHFUNCTION_H
