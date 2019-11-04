#include <cstddef>
#include "CuckooHash.h"

#include "city.h"

using namespace std;

template<typename element_type, typename fp_type>
uint128 CuckooHash<element_type, fp_type>::cityHashFunction(element_type *buff, size_t len) {
    char *buf = (char *) *buff;
    return CityHash128(buf, len);
}

template<typename element_type, typename fp_type>
fp_type CuckooHash<element_type, fp_type>::fingerprintFunction(element_type *element, size_t len) {
    char *buf = (char *) *element;
    return CityHash32(buf, len);
}
