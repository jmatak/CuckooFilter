#ifndef CUCKOOTEST_CUCKOOHASH_H
#define CUCKOOTEST_CUCKOOHASH_H

#include "city.h"

template<typename element_type, typename fp_type>
class CuckooHash {
public:
    static uint128 cityHashFunction(element_type *buff, size_t len);

    static fp_type fingerprintFunction(element_type *hash, size_t len);
};


#endif //CUCKOOTEST_CUCKOOHASH_H