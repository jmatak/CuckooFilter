#include <random>

#define A 54059
#define B 76963
#define FIRSTH 37

// Martin Dietzfelbinger, "Universal hashing and k-wise independent random
// variables via integer arithmetic without primes".
class HashFunction {
private:
    uint32_t multiply_, add_;

public:
    HashFunction() {
        ::std::random_device random;
        for (auto v : {&multiply_, &add_}) {
            *v = random();
        }
    }

    uint32_t hash(uint32_t key) const {
        return (add_ + multiply_ * static_cast<decltype(multiply_)>(key));
    }

    uint32_t hash(const std::string& key) const {
        const char *s = key.c_str();
        uint32_t h = FIRSTH;
        while (*s) {
            h = (h * A) ^ (s[0] * B);
            s++;
        }
        return h;
    }
};


static const uint32_t MURMUR_CONST = 0x5bd1e995;

inline static uint32_t fingerprintComplement(const size_t index, const uint32_t fp) {
    return index ^ (fp * MURMUR_CONST);
}