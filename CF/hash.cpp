#include <random>

template<typename element_type>
class HashFunction {
public:
    virtual uint32_t hash(element_type key) const = 0;
};

// Martin Dietzfelbinger, "Universal hashing and k-wise independent random
// variables via integer arithmetic without primes".
class MultiplyShift : public HashFunction<uint32_t>{
private:
    uint32_t multiply_, add_;

public:
    MultiplyShift() {
        ::std::random_device random;
        for (auto v : {&multiply_, &add_}) {
            *v = random();
        }
    }

    uint32_t hash(uint32_t key) const {
        return (add_ + multiply_ * static_cast<decltype(multiply_)>(key));
    }
};


static const uint32_t MURMUR_CONST = 0x5bd1e995;

inline static uint32_t fingerprintComplement(const size_t index, const uint32_t fp) {
    return index ^ (fp * MURMUR_CONST);
}