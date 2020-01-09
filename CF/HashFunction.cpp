#include "HashFunction.h"


HashFunction::HashFunction() {
    ::std::random_device random;
    for (auto v : {&multiply_, &add_}) {
        *v = random();
    }
}

uint32_t HashFunction::cityHashFunction(uint32_t *buff, size_t len) {
    char *buf = (char *) buff;
    return CityHash32(buf, len);
}


uint32_t HashFunction::cityHashFunction(std::string *buff, size_t len) {
    char *buf = (char *) buff;
    uint32_t h = CityHash32(buf, len);

    return h;
}


uint32_t HashFunction::hash(uint32_t key) const {
    return cityHashFunction(&key, (size_t) sizeof(uint32_t));
}

uint32_t HashFunction::hash(std::string key) const {
    return cityHashFunction(&key, (size_t) key.size());
}
