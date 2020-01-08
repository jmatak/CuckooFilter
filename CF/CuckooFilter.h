#ifndef CUCKOOFILTER_CUCKOOFILTER_H
#define CUCKOOFILTER_CUCKOOFILTER_H

#include <type_traits>
#include "CuckooTable.h"
#include "HashFunction.h"

#define KICKS_MAX_COUNT 500

static size_t highestPowerOfTwo(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    v >>= 1;
    return v;
}

struct Victim {
    uint32_t fp;
    size_t index;
};

template<typename element_type, typename fp_type = uint8_t>
class CuckooFilter {
private:
    uint32_t fp_mask{};
    CuckooTable<fp_type> *table;
    size_t element_count;
    HashFunction *hash_function;
    Victim victim;

    inline size_t getIndex(uint32_t hv) const;

    inline uint32_t fingerprint(uint32_t hash_value) const;

    inline void firstPass(const element_type &item, uint32_t *fp, size_t *index) const;

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const;

    bool insert(uint32_t fp, size_t index);

public:
    CuckooFilter(uint32_t max_table_size, size_t bits_per_fp = 8, size_t entries_per_bucket = 4);

    ~CuckooFilter();

    void print();

    bool insertElement(element_type &element);

    bool deleteElement(const element_type &element);

    bool containsElement(element_type &element);

    double availability();
};


#endif