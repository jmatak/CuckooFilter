#ifndef CUCKOOFILTER_HASH_FUNCTION_H
#define CUCKOOFILTER_HASH_FUNCTION_H


#include <random>
#include "city.h"
#include "murmur_hash3.h"

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

    uint64_t hash(std::string key) const;

    static uint64_t cityHashFunction(uint32_t *buff, size_t len);

    static uint64_t cityHashFunction(std::string *buff, size_t len);

    static uint64_t murmurHash3Function(std::string *buff, size_t len);
};


inline static uint32_t fingerprintComplement(const size_t index, const uint32_t fp) {
    return index ^ (fp * MURMUR_CONST);
}

#endif