#include "HashFunction.h"


HashFunction::HashFunction() {
    srand(1);
    for (auto v : {&multiply_, &add_}) {
        *v = rand();
        for (int i = 1; i <= 4; ++i) {
            *v = *v << 32;
            *v |= rand();
        }
    }
}
/**
 * CityHash hash function for uint type
 *
 * @param buff  Buffer for uint hash
 * @param len  Length of uint
 * @return Uint hash
 */
uint32_t HashFunction::cityHashFunction(uint32_t *buff, size_t len) {
    char *buf = (char *) buff;
    return CityHash32(buf, len);
}


/**
 * CityHash hash function for string type
 *
 * @param buff  Buffer for string hash
 * @param len  Length of string
 * @return String hash
 */
uint32_t HashFunction::cityHashFunction(std::string *buff, size_t len) {
    char *buf = (char *) buff;
    uint32_t h = CityHash32(buf, len);

    return h;
}

/**
 * Hash function for string keys
 * @param key Key of string type
 * @return Hash function for string
 */
uint32_t HashFunction::hash(std::string key) const {
    return cityHashFunction(&key, (size_t) key.size());
}



/**
 * Hash function for integer keys
 * @param key Key of integer type
 * @return Hash function for uint32
 */
uint64_t HashFunction::hash(uint32_t key) const {
    return (add_ + multiply_ * static_cast<decltype(multiply_)>(key)) >> 64;
}

