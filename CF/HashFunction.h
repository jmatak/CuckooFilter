#ifndef CUCKOOFILTER_HASHFUNCTION_H
#define CUCKOOFILTER_HASHFUNCTION_H


#include <random>
#include "city.h"

#define HASH_A 54059
#define HASH_B 76963
#define FIRSTH 37
static const uint32_t MURMUR_CONST = 0x5bd1e995;

// Martin Dietzfelbinger, "Universal hashing and k-wise independent random
// variables via integer arithmetic without primes".
class HashFunction {
private:
    unsigned __int128 multiply_, add_;


public:
    HashFunction();

    uint64_t hash(uint32_t key) const;

    //uint64_t fingerprint(uint32_t key) const;

    uint32_t hash(std::string key) const;

    //uint32_t fingerprint(std::string key) const;

    static uint32_t cityHashFunction(uint32_t *buff, size_t len);

    static uint32_t cityHashFunction(std::string *buff, size_t len);
};


inline static uint32_t fingerprintComplement(const size_t index, const uint32_t fp) {
    return index ^ (fp * MURMUR_CONST);
}

#endif