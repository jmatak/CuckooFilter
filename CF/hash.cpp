#include <random>

// Martin Dietzfelbinger, "Universal hashing and k-wise independent random
// variables via integer arithmetic without primes".
class MultiplyShift {
    uint32_t multiply_, add_;

public:
    MultiplyShift() {
        ::std::random_device random;
        for (auto v : {&multiply_, &add_}) {
            *v = random();
        }
    }


    uint32_t operator()(uint32_t key) const {
        return (add_ + multiply_ * static_cast<decltype(multiply_)>(key));
    }
};


static const uint32_t MURMUR_CONST = 0x5bd1e995;

inline static uint32_t fingerprintComplement(const size_t index, const uint32_t fp) {
    return index ^ (fp * MURMUR_CONST);
}


/**

template<typename element_type, typename fp_type>
uint128 hash<element_type, fp_type>::hashFunction(element_type *buff, size_t len) {
    char *buf = (char *) *buff;
    return CityHash128(buf, len);
}

template<typename element_type, typename fp_type>
fp_type hash<element_type, fp_type>::fingerprintFunction(element_type *element, size_t len) {
    char *buf = (char *) *element;
    return CityHash32(buf, len);
}
**/
