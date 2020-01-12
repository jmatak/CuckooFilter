#include "HashFunction.h"


//TODO: make the hashFunctions static


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

uint64_t HashFunction::cityHashFunction(uint64_t *buff, size_t len) {
    char *buf = (char *) buff;
    return CityHash64(buf, len);
}


uint64_t HashFunction::cityHashFunction(std::string *buff, size_t len) {
    char *buf = (char *) buff;
    uint64_t h = CityHash64(buf, len);

    return h;
}

uint64_t HashFunction::hash(std::string key) const {
    return cityHashFunction(&key, (size_t) key.size());
}

uint64_t HashFunction::hash(uint32_t key) const {
    return (add_ + multiply_ * static_cast<decltype(multiply_)>(key)) >> 64;
}
