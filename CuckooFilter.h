#ifndef CUCKOOTEST_CUCKOOFILTER_H
#define CUCKOOTEST_CUCKOOFILTER_H


#include "CuckooTable.h"
#include "CuckooHash.h"


template<typename element_type, typename fp_type>
class CuckooFilter {
    CuckooTable<fp_type> table;
private:
    fp_type fingerprintFunction(element_type element);

public:
    int hashFunction(fp_type fingerprint) {
        return murmurHash(&fingerprint, sizeof(fingerprint), 0);
    }

    explicit CuckooFilter(size_t tableSize);

    void insertElement(element_type element);

    void deleteElement(element_type element);

    bool contains(element_type element);
};


#endif
